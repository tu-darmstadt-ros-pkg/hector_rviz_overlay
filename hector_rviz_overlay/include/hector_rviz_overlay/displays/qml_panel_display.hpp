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

#ifndef HECTOR_RVIZ_OVERLAY_QML_PANEL_DISPLAY_H
#define HECTOR_RVIZ_OVERLAY_QML_PANEL_DISPLAY_H

#include <QStringList>

#include <rviz_common/display.hpp>

class QQmlEngine;
class QQuickView;
class QWidget;

namespace hector_rviz_overlay
{
class FileSystemWatcher;
class OgreGlContextGuard;
class QmlLoadedFileTracker;
class QmlRvizContext;

/*!
 * @class QmlPanelDisplay
 * @brief A base class for displays that show a QML file in a dockable panel.
 *
 * Classes that inherit from this class should implement the getPathToQml() method and return the
 * path to the qml file that should be loaded.
 *
 * Also, do not forget the Q_OBJECT macro, otherwise, your display may clash with other displays.
 */
class QmlPanelDisplay : public rviz_common::Display
{
  Q_OBJECT
public:
  QmlPanelDisplay();

  ~QmlPanelDisplay() override;

  void load( const rviz_common::Config &config ) override;

  void save( rviz_common::Config config ) const override;

  void update( float wall_dt, float ros_dt ) override;

  /*!
   * Adds the given directory path as import path to the QQmlEngine used for loading the Qml file.
   *
   * @param dir The path of the import directory.
   */
  void addImportPath( const QString &dir );

  /*!
   * Adds the given directory path as plugin path to the QQmlEngine used for loading the Qml file.
   *
   * @param dir The path of the plugin directory.
   */
  void addPluginPath( const QString &dir );

protected Q_SLOTS:

  //! Enables/disables the file system watcher and reloads.
  virtual void onLiveReloadChanged();

protected:
  void onInitialize() override;

  void onEnable() override;

  void onDisable() override;

  /*!
   * @return The path to the qml file, either absolute or package relative
   *   ("package://{package}/{path}"). An empty path clears the panel.
   */
  virtual QString getPathToQml();

  //! Schedules a reload on the event loop. Call after getPathToQml() changed.
  //! Never reloads synchronously because QML may have triggered the change.
  void requestReload();

  QmlRvizContext *rviz_context_ = nullptr;
  rviz_common::properties::BoolProperty *live_reload_property_;

private:
  void reload();

  void createQuickView( const QString &resolved_path );

  void destroyQuickView();

  void updateStatus();

  std::unique_ptr<OgreGlContextGuard> gl_context_guard_;
  std::unique_ptr<QWidget> container_;
  //! The window the QML scene renders in. Owned by view_container_.
  QQuickView *quick_view_ = nullptr;
  //! The widget embedding quick_view_ into the panel. Owns the view.
  QWidget *view_container_ = nullptr;
  QQmlEngine *engine_ = nullptr;
  std::unique_ptr<QmlLoadedFileTracker> file_tracker_;
  std::unique_ptr<FileSystemWatcher> file_system_watcher_;
  QStringList import_paths_;
  QStringList plugin_paths_;
  bool reload_requested_ = false;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_QML_PANEL_DISPLAY_H
