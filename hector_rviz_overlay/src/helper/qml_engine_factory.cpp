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

#include "qml_engine_factory.hpp"

#include "hector_rviz_overlay/helper/file_system_watcher.hpp"
#include "hector_rviz_overlay/helper/qml_rviz_context.hpp"
#include "hector_rviz_overlay/helper/rviz_tool_icon_provider.hpp"

#include <QQmlContext>
#include <QQmlEngine>

#include <rviz_common/display_context.hpp>

namespace hector_rviz_overlay
{

QUrl QmlLoadedFileTracker::intercept( const QUrl &path, QQmlAbstractUrlInterceptor::DataType type )
{
  if ( type != QmlFile && type != JavaScriptFile )
    return path;
  if ( !path.isLocalFile() )
    return path;
  std::string local_path = path.toLocalFile().toStdString();
  if ( watcher_ != nullptr )
    watcher_->addWatch( local_path );
  loaded_files_.push_back( local_path );
  return path;
}

void QmlLoadedFileTracker::setWatcher( FileSystemWatcher *watcher )
{
  watcher_ = watcher;
  if ( watcher_ == nullptr )
    return;
  for ( const std::string &path : loaded_files_ ) watcher_->addWatch( path );
}

void QmlLoadedFileTracker::clear() { loaded_files_.clear(); }

QQmlEngine *createRvizQmlEngine( const RvizQmlEngineConfig &config )
{
  auto *engine = new QQmlEngine( config.parent );

  if ( config.url_interceptor != nullptr )
    engine->setUrlInterceptor( config.url_interceptor );

  engine->rootContext()->setContextProperty( "rviz", config.rviz_context );

  auto *tool_icon_provider = new RvizToolIconProvider( config.display_context->getToolManager() );
  engine->addImageProvider( QLatin1String( "rviz_tool_icons" ), tool_icon_provider );

  for ( const auto &path : config.import_paths ) engine->addImportPath( path );
  for ( const auto &path : config.plugin_paths ) engine->addPluginPath( path );

  return engine;
}
} // namespace hector_rviz_overlay
