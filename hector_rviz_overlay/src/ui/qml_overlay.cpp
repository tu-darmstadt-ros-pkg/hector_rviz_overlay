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

#include "hector_rviz_overlay/ui/qml_overlay.h"

#include "hector_rviz_overlay/helper/file_system_watcher.h"
#include "hector_rviz_overlay/helper/qml_rviz_context.h"
#include "hector_rviz_overlay/helper/rviz_tool_icon_provider.h"
#include "hector_rviz_overlay/render/renderer.h"
#include "hector_rviz_overlay/overlay_manager.h"
#include "hector_rviz_overlay/path_helper.h"

#include <QApplication>
#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QQmlAbstractUrlInterceptor>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QQuickRenderControl>
#include <QQuickWindow>
#include <QWidget>

#include <rviz/display_context.h>

#include <ros/ros.h>
#include <chrono>

namespace hector_rviz_overlay
{

class QmlOverlay::UrlInterceptor : public QQmlAbstractUrlInterceptor
{
public:
  explicit UrlInterceptor( QmlOverlay *overlay ) : overlay_( overlay ) { }

  QUrl intercept( const QUrl &path, DataType type ) override
  {
    if ( type != QmlFile && type != JavaScriptFile ) return path;
    if ( !path.isLocalFile()) return path;
    std::string local_path = path.toLocalFile().toStdString();
    if ( overlay_->file_system_watcher_ != nullptr )
      overlay_->file_system_watcher_->addWatch( local_path );
    overlay_->loaded_files_.push_back( local_path );
    return path;
  }

private:
  QmlOverlay *overlay_;
};

QmlOverlay::QmlOverlay( const std::string &name )
  : UiOverlay( name )
{
  qRegisterMetaType<QmlOverlay::Status>();
  connect( this, &Overlay::visibilityChanged, this, &QmlOverlay::onVisibilityChanged );
}

QmlOverlay::~QmlOverlay()
{
  delete url_interceptor_;
  delete qml_rviz_context_;
}

void QmlOverlay::update( float dt )
{
  if ( !live_reload_enabled_ ) return;

  // Check the file for changes every 500ms.
  time_until_file_check_ -= dt;
  if ( time_until_file_check_ <= 0 )
  {
    if ( file_system_watcher_->checkForChanges())
    {
      reload_required_ = true;
      requestRender();
    }
    time_until_file_check_ = 0.5f;
  }
}

void QmlOverlay::prepareRender( Renderer *renderer )
{
  if ( render_control_ != nullptr ) return;

  QOpenGLContext *context;
  try
  {
    context = renderer->context();
  }
  catch ( std::logic_error &ex )
  {
    ROS_WARN( "Could not get renderer's context! Reason: %s", ex.what());
    setStatus( Error );
    return;
  }
  rviz_window_ = renderer->window();

  render_control_ = new QQuickRenderControl();
  quick_window_ = new QQuickWindow( render_control_ );

  engine_ = new QQmlEngine( quick_window_ );

  delete url_interceptor_;
  url_interceptor_ = new UrlInterceptor( this );
  engine_->setUrlInterceptor( url_interceptor_ );
  if ( !engine_->incubationController())
    engine_->setIncubationController( quick_window_->incubationController());
  qml_rviz_context_ = new QmlRvizContext( OverlayManager::getSingleton().displayContext(), this, false );
  qml_rviz_context_->setConfig( configuration_ );
  emit contextCreated();
  engine_->rootContext()->setContextProperty( "rviz", qml_rviz_context_ );
  auto *tool_icon_provider = new RvizToolIconProvider(
    OverlayManager::getSingleton().displayContext()->getToolManager());
  engine_->addImageProvider( QLatin1String( "rviz_tool_icons" ), tool_icon_provider );

  component_ = new QQmlComponent( engine_ );

  quick_window_->contentItem()->setTransformOrigin( QQuickItem::TopLeft );
  quick_window_->contentItem()->setScale( scale());
  quick_window_->setColor( Qt::transparent );
  quick_window_->setClearBeforeRendering( false );
  quick_window_->setPersistentOpenGLContext( true );

  if ( !path_.isEmpty())
  {
    load( path_ );
  }

  render_control_->initialize( context );

  connect( render_control_, &QQuickRenderControl::sceneChanged, this, &QmlOverlay::onSceneChanged );
  connect( render_control_, &QQuickRenderControl::renderRequested, this, &QmlOverlay::onRenderRequested );

  if ( isVisible())
  {
    QShowEvent event;
    QApplication::sendEvent( quick_window_, &event );
    qml_rviz_context_->setVisible( true );
  }
}

void QmlOverlay::releaseRenderResources()
{
  delete render_control_;
  qml_rviz_context_ = nullptr;
  render_control_ = nullptr;
  component_ = nullptr;
  quick_window_ = nullptr;
  engine_ = nullptr;
}

void QmlOverlay::renderImpl( Renderer *renderer )
{
  if ( render_control_ == nullptr ) return;

  if ( reload_required_ )
  {
    reload();
  }

  if ( quick_window_->renderTarget() != renderer->framebufferObject() || renderer->framebufferObjectUpdated())
  {
    quick_window_->setRenderTarget( renderer->framebufferObject());
  }
  if ( scene_changed_ )
  {
    // According to https://youtu.be/FamAKvkfzxM?t=2247, polish and sync should be called when QQuickRenderControl emits sceneUpdated
    render_control_->polishItems();
    render_control_->sync();
    scene_changed_ = false;
  }
  render_control_->render();
  renderer->context()->functions()->glFlush();
}

namespace
{
bool sendKeyEventToFocusedItems( QObject *obj, QEvent *event )
{
  auto *item = qobject_cast<QQuickItem *>( obj );
  if ( !item )
  {
    for ( auto &child : obj->children())
    {
      if ( sendKeyEventToFocusedItems( child, event )) return true;
    }
    return event->isAccepted();
  }
  for ( auto &child : item->childItems())
  {
    if ( sendKeyEventToFocusedItems( child, event )) return true;
  }

  if ( !item->hasFocus()) return event->isAccepted();
  // Check that it isnt in an inactive focus scope
  auto *current = item;
  while ( current != nullptr )
  {
    if ( current->isFocusScope() && current->parentItem() != nullptr && !current->hasFocus())
      return event->isAccepted();
    current = current->parentItem();
  }
  // It is not, so deliver to it
  current = item;
  while ( current != nullptr )
  {
    QCoreApplication::sendEvent( current, event );
    if ( event->isAccepted()) break;
    current = current->parentItem();
  }
//  if ( current != nullptr ) ROS_INFO_STREAM( current->metaObject()->className() << " accepted" );
  return event->isAccepted();
}
}

bool QmlOverlay::handleEvent( QObject *, QEvent *event )
{
  if ( render_control_ == nullptr ) return false;
  if ( sending_event_ ) return false;
  sending_event_ = true;
  event->ignore();

  if ( event->type() != QEvent::KeyPress && event->type() != QEvent::KeyRelease )
  {
    QCoreApplication::sendEvent( quick_window_, event );
    if ( event->isAccepted()) quick_window_->contentItem()->setFocus( true );
  }
  else
  {
    sendKeyEventToFocusedItems( quick_window_, event );
//    ROS_INFO("Event was accepted: %s", event->isAccepted() ? "true" : "false");
  }
  // A bit of a hack, if the overlay changed the cursor, we consume the mouse move events to prevent rviz from overwriting it
  if ( rviz_window_ != nullptr && quick_window_->cursor().shape() != Qt::ArrowCursor )
  {
    event->accept();
    rviz_window_->setCursor( quick_window_->cursor());
  }
  sending_event_ = false;
  return event->isAccepted();
}

void QmlOverlay::setGeometry( const QRect &value )
{
  Overlay::setGeometry( value );
  if ( quick_window_ == nullptr ) return;

  updateGeometry();
}

void QmlOverlay::setScale( float value )
{
  Overlay::setScale( value );
  if ( quick_window_ == nullptr ) return;

  quick_window_->contentItem()->setScale( value );
  updateGeometry();
}

void QmlOverlay::updateGeometry()
{
  if ( root_item_ == nullptr ) return;
  if ( geometry().topLeft() != quick_window_->geometry().topLeft())
  {
    quick_window_->setPosition( geometry().topLeft());
  }
  QSize old_size = quick_window_->size();
  QSizeF new_size = QSizeF( geometry().size()) / scale();

  quick_window_->resize( std::round( new_size.width()), std::round( new_size.height()));
  quick_window_->contentItem()->setSize( new_size );
  root_item_->setSize( new_size );
  QResizeEvent resize_event( quick_window_->size(), old_size );
  QCoreApplication::sendEvent( quick_window_, &resize_event );
  scene_changed_ = true;
  requestRender();
}

void QmlOverlay::handleEventsCanceled()
{
  // TODO
}

bool QmlOverlay::load( const QString &path )
{
  path_ = path;
  return createRootItem();
}

bool QmlOverlay::reload()
{
  reload_required_ = false;
  if ( engine_ == nullptr )
  {
    if ( status() != Error ) setStatus( Uninitialized );
    return false;
  }
  ROS_INFO( "Reloading qml file." );
  engine_->clearComponentCache();
  return createRootItem();
}

bool QmlOverlay::createRootItem()
{
  if ( engine_ == nullptr )
  {
    if ( status() != Error ) setStatus( Uninitialized );
    return false;
  }
  // Reset watches as the ones needed will be added on creation
  if ( live_reload_enabled_ && !path_.isEmpty())
  {
    file_system_watcher_->removeAllWatches();
  }
  loaded_files_.clear();

  // Component has to be deleted and recreated otherwise it won't reload if the last load failed
  delete component_;
  component_ = new QQmlComponent( engine_ );

  component_->loadUrl( QUrl( resolvePath( path_ )));
  if ( component_->isError())
  {
    ROS_WARN_STREAM( "Error while trying to load QML: " << component_->errorString().toStdString());
    setStatus( LoadingFailed );
    return false;
  }

  QObject *root_object = component_->create();
  if ( component_->isError())
  {
    ROS_WARN_STREAM( "Error while trying to create QML component: " << component_->errorString().toStdString());
    setStatus( CreationFailed );
    return false;
  }

  auto *root_item = qobject_cast<QQuickItem *>( root_object );
  if ( !root_item )
  {
    setStatus( Error );
    return false;
  }

  delete root_item_;
  root_item_ = root_item;

  root_item_->setParentItem( quick_window_->contentItem());
  root_item_->setParent( quick_window_->contentItem());
  root_item_->setTransformOrigin( QQuickItem::TopLeft );
  updateGeometry();
  setStatus( Ok );
  // Have to manually set scene changed since the reload won't trigger an event
  scene_changed_ = true;
  requestRender();
  return true;
}

QmlOverlay::Status QmlOverlay::status()
{
  return status_;
}

void QmlOverlay::setStatus( QmlOverlay::Status status )
{
  status_ = status;
  emit statusChanged( status_ );
}

void QmlOverlay::addImportPath( const QString &dir )
{
  engine_->addImportPath( dir );
}

void QmlOverlay::addPluginPath( const QString &dir )
{
  engine_->addPluginPath( dir );
}

void QmlOverlay::setLiveReloadEnabled( bool value )
{
  if ( live_reload_enabled_ == value ) return;
  live_reload_enabled_ = value;
  if ( live_reload_enabled_ )
  {
    // TODO: Check valid and check if watch was successful + display status
    file_system_watcher_.reset( new FileSystemWatcher );
    for ( const std::string &path : loaded_files_ )
      file_system_watcher_->addWatch( path );
    reload();
  }
  else
  {
    file_system_watcher_.reset();
  }
}

bool QmlOverlay::liveReloadEnabled() const { return live_reload_enabled_; }

const QVariantMap &QmlOverlay::configuration() const
{
  if ( engine_ != nullptr )
  {
    if ( qml_rviz_context_ != nullptr ) return qml_rviz_context_->config();
  }
  return configuration_;
}

void QmlOverlay::setConfiguration( const QVariantMap &value )
{
  configuration_ = value;
  if ( engine_ != nullptr )
  {
    if ( qml_rviz_context_ != nullptr ) qml_rviz_context_->setConfig( value );
  }
}

void QmlOverlay::onSceneChanged()
{
  scene_changed_ = true;
  requestRender();
}

void QmlOverlay::onRenderRequested()
{
  requestRender();
}

void QmlOverlay::onVisibilityChanged()
{
  if ( quick_window_ != nullptr )
  {
    if ( isVisible())
    {
      QShowEvent event;
      QApplication::sendEvent( quick_window_, &event );
    }
    else
    {
      QHideEvent event;
      QApplication::sendEvent( quick_window_, &event );
    }
  }
  if ( qml_rviz_context_ == nullptr ) return;
  qml_rviz_context_->setVisible( isVisible());
}

QmlRvizContext *QmlOverlay::context()
{
  return qml_rviz_context_;
}

const QQmlEngine *QmlOverlay::engine() const
{
  return engine_;
}

QQmlEngine *QmlOverlay::engine()
{
  return engine_;
}

const QQuickItem *QmlOverlay::rootItem() const
{
  return root_item_;
}

QQuickItem *QmlOverlay::rootItem()
{
  return root_item_;
}
}
