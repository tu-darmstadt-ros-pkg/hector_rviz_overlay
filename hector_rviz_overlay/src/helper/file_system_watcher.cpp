/*
 * Copyright (C) 2019  Stefan Fabian
 *
 * This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "hector_rviz_overlay/helper/file_system_watcher.hpp"

#include <filesystem>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

#include "../logging.hpp"

namespace hector_rviz_overlay
{

FileSystemWatcher::FileSystemWatcher() { file_descriptor_ = inotify_init1( IN_NONBLOCK ); }

FileSystemWatcher::~FileSystemWatcher()
{
  for ( const auto &[_, wd] : watched_directories_ ) { inotify_rm_watch( file_descriptor_, wd ); }
  if ( file_descriptor_ >= 0 )
    close( file_descriptor_ );
}

bool FileSystemWatcher::isValid() const { return file_descriptor_ >= 0; }

bool FileSystemWatcher::addWatch( const std::string &path )
{
  if ( path.empty() )
    return false;
  if ( std::find( watched_paths_.begin(), watched_paths_.end(), path ) != watched_paths_.end() ) {
    // Already watching this path, just add another reference
    watched_paths_.push_back( path );
    return true;
  }
  if ( std::filesystem::is_symlink( path ) ) {
    // Follow symlinks
    std::error_code error;
    std::string target = std::filesystem::read_symlink( path, error );
    if ( error ) {
      LOG_WARN( "OverlayManager: Could not read link '%s' (%s)!", path.c_str(),
                error.message().c_str() );
      return false;
    }
    return addWatch( target );
  }
  const bool is_directory = std::filesystem::is_directory( path );
  if ( !is_directory && !std::filesystem::is_regular_file( path ) ) {
    return false;
  }

  std::string::size_type separator_pos = path.find_last_of( '/' );
  std::filesystem::path p( path );
  if ( !p.has_parent_path() )
    return false;
  std::string folder = is_directory ? path : p.parent_path().string();

  auto it = watched_directories_.find( folder );
  if ( it == watched_directories_.end() ) {
    int watch = inotify_add_watch( file_descriptor_, folder.c_str(),
                                   IN_CREATE | IN_DELETE | IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO );
    if ( watch < 0 )
      return false;
    if ( is_directory ) {
      // Include subdirectories
      for ( const auto &dir : std::filesystem::directory_iterator(
                folder, std::filesystem::directory_options::follow_directory_symlink ) ) {
        if ( !std::filesystem::is_directory( dir.path() ) )
          continue;
        if ( !addWatch( dir.path().string() ) ) {
          LOG_WARN( "OverlayManager: Could not add file system watch for '%s'!", dir.path().c_str() );
        }
      }
    }
    it = watched_directories_.try_emplace( folder, watch ).first;
    watch_info_.insert( { watch, { is_directory, {} } } );
  }
  watched_paths_.push_back( path );
  WatchInfo &info = watch_info_.at( it->second );
  info.is_directory |= is_directory;
  if ( !is_directory )
    info.filenames.insert( path.substr( separator_pos + 1 ) );
  return true;
}

void FileSystemWatcher::removeWatch( const std::string &path )
{
  // Resolve symlinks first, mirroring addWatch which records the resolved target rather than the
  // link. Decrementing on the link key would miss the target's reference and leak its watch.
  if ( std::filesystem::is_symlink( path ) ) {
    std::error_code error;
    std::string target = std::filesystem::read_symlink( path, error );
    if ( error ) {
      LOG_WARN( "OverlayManager: Could not read link '%s' (%s) when removing watch!", path.c_str(),
                error.message().c_str() );
      return;
    }
    removeWatch( target );
    return;
  }

  auto path_it = std::find( watched_paths_.begin(), watched_paths_.end(), path );
  if ( path_it == watched_paths_.end() )
    return;
  watched_paths_.erase( path_it );
  // Only unregister if no other references to this path are left
  if ( std::find( watched_paths_.begin(), watched_paths_.end(), path ) != watched_paths_.end() )
    return;

  // Check if folder
  if ( auto it = watched_directories_.find( path ); it != watched_directories_.end() ) {
    // Remove subfolder watches
    for ( const auto &dir : std::filesystem::directory_iterator( path ) ) {
      if ( !std::filesystem::is_directory( dir.path() ) || path == dir.path().string() )
        continue;
      removeWatch( dir.path().string() );
    }
    // This watch is not for a directory anymore but may still be needed for a file
    watch_info_.at( it->second ).is_directory = false;
    removeFolderWatchIfNoFilesLeft( path );
    return;
  }

  std::string::size_type separator_pos = path.find_last_of( '/' );
  if ( separator_pos == std::string::npos )
    return;

  std::string folder = path.substr( 0, separator_pos );
  std::string filename = path.substr( separator_pos + 1 );
  watch_info_.at( watched_directories_.at( folder ) ).filenames.erase( filename );
  removeFolderWatchIfNoFilesLeft( folder );
}

void FileSystemWatcher::removeAllWatches()
{
  for ( const auto &[_, wd] : watched_directories_ ) { inotify_rm_watch( file_descriptor_, wd ); }
  watched_directories_.clear();
  watched_paths_.clear();
  watch_info_.clear();
}

bool FileSystemWatcher::checkForChanges() const
{
  const size_t buffer_length = 16 * sizeof( struct inotify_event );
  unsigned char buffer[buffer_length];
  bool changed = false;
  ssize_t count;
  while ( ( count = read( file_descriptor_, buffer, buffer_length ) ) > 0 ) {
    if ( changed )
      continue;
    size_t offset = 0;
    const struct inotify_event *event;
    while ( offset < static_cast<size_t>( count ) ) {
      event = reinterpret_cast<const inotify_event *>( buffer + offset );
      offset += sizeof( struct inotify_event ) + event->len;

      // Nameless events (IN_IGNORED, IN_Q_OVERFLOW, IN_UNMOUNT) are delivered regardless of the
      // requested mask and carry no name; reading event->name would over-read past the payload.
      if ( event->len == 0 )
        continue;

      // Ignore qmlc file changes because we create them. Some of them are temps and end in .qmlc.[RANDOMSTRING]
      if ( std::string name = event->name;
           name.find( ".qmlc." ) != std::string::npos ||
           ( name.length() > 5 && name.substr( name.length() - 5 ) == ".qmlc" ) ) {
        continue;
      }

      // Check if the change was on a watched file
      auto it = watch_info_.find( event->wd );
      if ( it == watch_info_.end() )
        continue;
      if ( it->second.is_directory ) {
        changed = true;
      } else if ( const auto &filenames = it->second.filenames;
                  event->len && filenames.find( event->name ) != filenames.end() ) {
        // Check if it was an action that might change the files content.
        changed = event->mask & ( IN_MODIFY | IN_MOVED_TO | IN_CLOSE_WRITE );
      }
      if ( changed )
        break;
    }
  }
  return changed;
}

void FileSystemWatcher::removeFolderWatchIfNoFilesLeft( const std::string &path )
{
  for ( const auto &file : watched_paths_ ) {
    if ( file.length() < path.length() || file.substr( 0, path.length() ) != path )
      continue;
    return;
  }
  auto it = watched_directories_.find( path );
  inotify_rm_watch( file_descriptor_, it->second );
  watch_info_.erase( it->second );
  watched_directories_.erase( it );
}
} // namespace hector_rviz_overlay
