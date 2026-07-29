/*
 * Copyright (C) 2025  Stefan Fabian
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

#ifndef HECTOR_RVIZ_OVERLAY_QML_ENGINE_FACTORY_H
#define HECTOR_RVIZ_OVERLAY_QML_ENGINE_FACTORY_H

#include <QQmlAbstractUrlInterceptor>
#include <QStringList>

#include <string>
#include <vector>

class QQmlEngine;

namespace rviz_common
{
class DisplayContext;
}

namespace hector_rviz_overlay
{
class FileSystemWatcher;
class QmlRvizContext;

//! Records the QML and JavaScript files an engine loads, so they can be watched for live reloading.
class QmlLoadedFileTracker : public QQmlAbstractUrlInterceptor
{
public:
  QUrl intercept( const QUrl &path, DataType type ) override;

  /*!
   * While a watcher is set, every intercepted file is added to it as it is loaded, and setting a
   * watcher immediately adds all files recorded so far.
   * The caller must call setWatcher( nullptr ) before destroying the watcher.
   */
  void setWatcher( FileSystemWatcher *watcher );

  //! Forgets the recorded files. Call whenever the watches are reset.
  void clear();

private:
  FileSystemWatcher *watcher_ = nullptr;
  std::vector<std::string> loaded_files_;
};

struct RvizQmlEngineConfig {
  QObject *parent = nullptr;
  rviz_common::DisplayContext *display_context = nullptr;
  //! Exposed to QML as the "rviz" context property. Not owned by the engine.
  QmlRvizContext *rviz_context = nullptr;
  QQmlAbstractUrlInterceptor *url_interceptor = nullptr;
  QStringList import_paths;
  QStringList plugin_paths;
};

//! Creates a QQmlEngine set up for rviz: "rviz" context property, "rviz_tool_icons" image
//! provider, url interceptor and the given import and plugin paths.
QQmlEngine *createRvizQmlEngine( const RvizQmlEngineConfig &config );
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_QML_ENGINE_FACTORY_H
