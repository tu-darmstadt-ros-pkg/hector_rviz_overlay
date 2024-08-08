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

#ifndef HECTOR_RVIZ_OVERLAY_FILE_WATCHER_H
#define HECTOR_RVIZ_OVERLAY_FILE_WATCHER_H

#include <map>
#include <string>
#include <set>
#include <vector>

namespace hector_rviz_overlay
{

class FileSystemWatcher
{
  struct WatchInfo
  {
    bool is_directory;
    std::set<std::string> filenames;
  };

public:
  FileSystemWatcher();

  ~FileSystemWatcher();

  bool isValid() const;

  bool addWatch( const std::string &path );

  void removeWatch( const std::string &path );

  void removeAllWatches();

  bool checkForChanges() const;

private:
  void removeFolderWatchIfNoFilesLeft( const std::string &path );

  int file_descriptor_ = -1;
  std::map<std::string, int> watched_directories_;
  std::vector<std::string> watched_paths_;
  /*!
   * Map from folder handle to watched file
   */
  std::map<int, WatchInfo> watch_info_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_FILE_WATCHER_H
