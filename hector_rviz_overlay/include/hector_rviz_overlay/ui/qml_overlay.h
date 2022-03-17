/*
 * Copyright (C) 2017  Stefan Fabian
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

#ifndef HECTOR_RVIZ_OVERLAY_QML_OVERLAY_H
#define HECTOR_RVIZ_OVERLAY_QML_OVERLAY_H

#include "ui_overlay.h"

#include <QVariantMap>

class QQmlComponent;
class QQmlEngine;
class QQuickItem;
class QQuickRenderControl;
class QQuickWindow;
class QWindow;

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace hector_rviz_overlay
{

class FileSystemWatcher;

class QmlRvizContext;

/*!
 * @class QmlOverlay
 * @brief This class allows to load and display a qml file.
 */
class QmlOverlay : public UiOverlay
{
Q_OBJECT
public:
  enum Status
  {
    Ok = 0,
    Uninitialized,
    LoadingFailed,
    CreationFailed,
    Error
  };

  explicit QmlOverlay( const std::string &name );

  ~QmlOverlay() override;

  void update( float dt ) override;

  /// @inherit
  void prepareRender( Renderer *renderer ) override;

  /// @inherit
  void releaseRenderResources() override;

  /// @inherit
  Q_INVOKABLE bool handleEvent( QObject *receiver, QEvent *event ) override;

  /// @inherit
  void handleEventsCanceled() override;

  /// @inherit
  void setScale( float value ) override;

  /// @inherit
  void setGeometry( const QRect &value ) override;

  /*!
   * Loads the qml file from the provided path.
   * The path can be specified either as an absolute path or a package relative path.
   * A package relative path has the following format: "package://{package_name}/{relative_path_in_package}".
   *
   * @param path The path to the qml file.
   * @return True if loading the qml file was successful, false otherwise
   */
  bool load( const QString &path );

  /*!
   * Reloads the qml file from the provided path.
   *
   * @return True if loading the qml file was successful, false otherwise
   */
  bool reload();

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

  /*!
   * If the status changes, the statusChanged signal is emitted.
   * @return The status of the overlay
   */
  QmlOverlay::Status status();

  /*!
   * Sets whether or not the loaded qml file should be watched for changes and reloaded if something changed.
   * Useful for development.
   * Default: false.
   * @param value True to enable live reload, false to disable it.
   */
  void setLiveReloadEnabled( bool value );

  /*!
   * See setLiveReloadEnabled.
   * @return Whether or not live reloading is enabled.
   */
  bool liveReloadEnabled() const;

  QmlRvizContext *context();

  const QVariantMap &configuration() const;

  void setConfiguration( const QVariantMap &value );

  const QQmlEngine *engine() const;

  QQmlEngine *engine();

signals:

  void statusChanged( hector_rviz_overlay::QmlOverlay::Status status );

  void contextCreated();

protected slots:
  void onSceneChanged();

  void onRenderRequested();

  void onVisibilityChanged();

protected:

  bool createRootItem();

  void updateGeometry();

  /// @inherit
  void renderImpl( Renderer *renderer ) override;

  void setStatus( QmlOverlay::Status status );

  class UrlInterceptor;
  UrlInterceptor *url_interceptor_ = nullptr;
  std::vector<std::string> loaded_files_;

  QWindow *rviz_window_;
  QQuickRenderControl *render_control_ = nullptr;
  QQuickWindow *quick_window_ = nullptr;
  QQmlEngine *engine_ = nullptr;
  QQmlComponent *component_ = nullptr;
  QQuickItem *root_item_ = nullptr;
  QVariantMap configuration_;
  QmlRvizContext *qml_rviz_context_ = nullptr;

  bool reload_required_ = false;
  bool scene_changed_ = true;
  bool sending_event_ = false;

  QString path_;
  Status status_ = Uninitialized;
  bool live_reload_enabled_ = false;
  std::unique_ptr<FileSystemWatcher> file_system_watcher_;
  float time_until_file_check_ = 0.5f;
};

typedef std::shared_ptr<QmlOverlay> QmlOverlayPtr;
typedef std::shared_ptr<const QmlOverlay> QmlOverlayConstPtr;
}

Q_DECLARE_METATYPE( hector_rviz_overlay::QmlOverlay::Status );

#endif //HECTOR_RVIZ_OVERLAY_QML_OVERLAY_H
