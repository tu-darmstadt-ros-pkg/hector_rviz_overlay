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

#include "hector_rviz_overlay/helper/file_system_watcher.h"

#include <boost/filesystem.hpp>
#include <ros/ros.h>
#include <sys/inotify.h>
#include <sys/stat.h>
#include <unistd.h>

namespace hector_rviz_overlay
{

FileSystemWatcher::FileSystemWatcher()
{
  file_descriptor_ = inotify_init1( IN_NONBLOCK );
}

FileSystemWatcher::~FileSystemWatcher()
{
  for ( auto &kv : watched_directories_ )
  {
    inotify_rm_watch( file_descriptor_, kv.second );
  }
  if ( file_descriptor_ >= 0 ) close( file_descriptor_ );
}

bool FileSystemWatcher::isValid() const
{
  return file_descriptor_ >= 0;
}

bool FileSystemWatcher::addWatch( const std::string &path )
{
  if ( path.length() == 0 ) return false;
  if ( std::find( watched_paths_.begin(), watched_paths_.end(), path ) != watched_paths_.end())
  {
    watched_paths_.push_back( path );
    return true;
  }
  struct stat path_stat;
  if ( stat( path.c_str(), &path_stat ) != 0 || (path_stat.st_mode & (S_IFDIR | S_IFREG)) == 0 ) return false;

  const bool is_directory = (path_stat.st_mode & S_IFDIR) == S_IFDIR;

  std::string::size_type separator_pos = path.find_last_of( '/' );
  if ( separator_pos == std::string::npos ) return false;
  std::string folder = is_directory ? path : path.substr( 0, separator_pos );

  auto it = watched_directories_.find( folder );
  if ( it == watched_directories_.end())
  {
    int watch = inotify_add_watch( file_descriptor_, folder.c_str(),
                                   IN_CREATE | IN_DELETE | IN_MODIFY | IN_CLOSE_WRITE | IN_MOVED_TO );
    if ( watch < 0 ) return false;
    if ( is_directory )
    {
      // Include subdirectories
      for ( const auto &dir : boost::filesystem::directory_iterator( folder ))
      {
        if ( !boost::filesystem::is_directory( dir.path())) continue;
        if ( !addWatch( dir.path().string()))
        {
          ROS_WARN_NAMED( "OverlayManager", "Could not add file system watch for '%s'!", dir.path().c_str());
        }
      }
    }
    it = watched_directories_.insert( { folder, watch } ).first;
    watch_info_.insert( { watch, { .is_directory = is_directory }} );
  }
  watched_paths_.push_back( path );
  WatchInfo &info = watch_info_.at( it->second );
  info.is_directory |= is_directory;
  if ( !is_directory ) info.filenames.insert( path.substr( separator_pos + 1 ));
  return true;
}

void FileSystemWatcher::removeWatch( const std::string &path )
{
  watched_paths_.erase( std::find( watched_paths_.begin(), watched_paths_.end(), path ));
  if ( std::find( watched_paths_.begin(), watched_paths_.end(), path ) != watched_paths_.end()) return;

  // Check if folder
  auto it = watched_directories_.find( path );
  if ( it != watched_directories_.end())
  {
    // Remove subfolder watches
    for ( const auto &dir : boost::filesystem::directory_iterator( path ))
    {
      if ( !boost::filesystem::is_directory( dir.path())) continue;
      removeWatch( dir.path().string());
    }
    // This watch is not for a directory anymore but may still be needed for a file
    watch_info_.at( it->second ).is_directory = false;
    removeFolderWatchIfNoFilesLeft( path );
    return;
  }

  std::string::size_type separator_pos = path.find_last_of( '/' );
  if ( separator_pos == std::string::npos ) return;

  std::string folder = path.substr( 0, separator_pos );
  std::string filename = path.substr( separator_pos + 1 );
  watch_info_.at( watched_directories_.at( folder )).filenames.erase( filename );
  removeFolderWatchIfNoFilesLeft( folder );
}

void FileSystemWatcher::removeAllWatches()
{
  for ( auto &kv : watched_directories_ )
  {
    inotify_rm_watch( file_descriptor_, kv.second );
  }
  watched_directories_.clear();
  watched_paths_.clear();
  watch_info_.clear();
}

bool FileSystemWatcher::checkForChanges() const
{
  const size_t buffer_length = 16 * sizeof( struct inotify_event );
  static unsigned char buffer[buffer_length];
  bool changed = false;
  ssize_t count;
  while ((count = read( file_descriptor_, buffer, buffer_length )) > 0 )
  {
    if ( changed ) continue;
    size_t offset = 0;
    struct inotify_event *event;
    while ( offset < count )
    {
      event = reinterpret_cast<inotify_event *>(buffer + offset);
      offset += sizeof( struct inotify_event ) + event->len;

      // Ignore qmlc file changes because we create them. Some of them are temps and end in .qmlc.[RANDOMSTRING]
      std::string name = event->name;
      if ( name.find( ".qmlc." ) != std::string::npos ||
           (name.length() > 5 && name.substr( name.length() - 5 ) == ".qmlc"))
      {
        continue;
      }

      // Check if the change was on a watched file
      auto it = watch_info_.find( event->wd );
      if ( it == watch_info_.end()) continue;
      if ( it->second.is_directory )
      {
        changed = true;
        break;
      }
      const std::set<std::string> &filenames = it->second.filenames;
      if ( event->len && filenames.find( event->name ) != filenames.end())
      {
        // Check if it was an action that might change the files content.
        if ( event->mask & (IN_MODIFY | IN_MOVED_TO | IN_CLOSE_WRITE))
        {
          changed = true;
          break;
        }
      }
    }
  }
  return changed;
}

void FileSystemWatcher::removeFolderWatchIfNoFilesLeft( const std::string &path )
{
  for ( auto &file : watched_paths_ )
  {
    if ( file.length() < path.length() || file.substr( 0, path.length()) != path ) continue;
    return;
  }
  auto it = watched_directories_.find( path );
  inotify_rm_watch( file_descriptor_, it->second );
  watch_info_.erase( it->second );
  watched_directories_.erase( it );
}
}
