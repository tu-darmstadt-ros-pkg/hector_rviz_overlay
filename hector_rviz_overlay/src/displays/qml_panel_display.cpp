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

#include "hector_rviz_overlay/displays/qml_panel_display.hpp"

#include "hector_rviz_overlay/helper/file_system_watcher.hpp"
#include "hector_rviz_overlay/helper/qml_rviz_context.hpp"
#include "hector_rviz_overlay/path_helper.hpp"

#include "../helper/config_conversion.hpp"
#include "../helper/qml_engine_factory.hpp"
#include "../render/ogre_gl_context.hpp"

#include <QApplication>
#include <QPalette>
#include <QQmlEngine>
#include <QQmlError>
#include <QQuickItem>
#include <QQuickView>
#include <QVBoxLayout>
#include <QWidget>

#include <rviz_common/properties/bool_property.hpp>
#include <rviz_common/properties/status_property.hpp>

using namespace rviz_common::properties;

namespace hector_rviz_overlay
{

namespace
{
/*!
 * The widget a window container is embedded in.
 *
 * The widget returned by QWidget::createWindowContainer has no size hint of its own, so without this
 * the dock the panel lives in would not know the size the qml file asks for.
 */
class PanelContainer : public QWidget
{
public:
  QSize sizeHint() const override
  {
    if ( view_ == nullptr )
      return QWidget::sizeHint();
    // Not sizeHint() because the view is embedded before the file is loaded, so SizeRootObjectToView
    // has already resized the root object and sizeHint() would only return the current size.
    const QSize size = view_->initialSize();
    // The same fallback QQuickWidget used for a root object without an implicit size.
    return size.isEmpty() ? QSize( 75, 75 ) : size;
  }

  void setView( QQuickView *view )
  {
    view_ = view;
    updateGeometry();
  }

private:
  QQuickView *view_ = nullptr;
};
} // namespace

QmlPanelDisplay::QmlPanelDisplay() : file_tracker_( std::make_unique<QmlLoadedFileTracker>() )
{
  // Both the property and the tracker have to exist before Display::load() runs because loading a
  // config with live reload enabled invokes onLiveReloadChanged().
  live_reload_property_ = new BoolProperty( "Live Reload", false,
                                            "Watches the qml file and reloads it if it changed.\n"
                                            " Useful for development.",
                                            this, SLOT( onLiveReloadChanged() ) );
}

QmlPanelDisplay::~QmlPanelDisplay()
{
  file_tracker_->setWatcher( nullptr );
  destroyQuickView();
  delete engine_;
  delete rviz_context_;
}

void QmlPanelDisplay::onInitialize()
{
  container_ = std::make_unique<PanelContainer>();
  auto *layout = new QVBoxLayout( container_.get() );
  layout->setContentsMargins( 0, 0, 0, 0 );

  rviz_context_ = new QmlRvizContext( context_, nullptr, isEnabled() );
  rviz_context_->setConfigurationPropertyParent( this );

  // The QQuickView renders on Qt Quick's render thread with its own context and does not touch
  // Ogre's. Only if Qt Quick falls back to rendering on the GUI thread, e.g. QSG_RENDER_LOOP=basic
  // or software rendering, its context is left current there, which the guard restores.
  gl_context_guard_ = std::make_unique<OgreGlContextGuard>( context_ );

  setAssociatedWidget( container_.get() );

  requestReload();
}

void QmlPanelDisplay::onEnable() { rviz_context_->setVisible( true ); }

void QmlPanelDisplay::onDisable() { rviz_context_->setVisible( false ); }

QString QmlPanelDisplay::getPathToQml() { return QString(); }

void QmlPanelDisplay::update( float, float )
{
  if ( file_system_watcher_ == nullptr )
    return;
  if ( !file_system_watcher_->checkForChanges() )
    return;
  requestReload();
}

void QmlPanelDisplay::addImportPath( const QString &dir )
{
  if ( !import_paths_.contains( dir ) )
    import_paths_.append( dir );
  if ( engine_ != nullptr )
    engine_->addImportPath( dir );
}

void QmlPanelDisplay::addPluginPath( const QString &dir )
{
  if ( !plugin_paths_.contains( dir ) )
    plugin_paths_.append( dir );
  if ( engine_ != nullptr )
    engine_->addPluginPath( dir );
}

void QmlPanelDisplay::onLiveReloadChanged()
{
  if ( live_reload_property_->getBool() ) {
    file_system_watcher_ = std::make_unique<FileSystemWatcher>();
    // Back-fills the watcher with the files that were already loaded, hence, no reload is needed.
    file_tracker_->setWatcher( file_system_watcher_.get() );
    return;
  }
  file_tracker_->setWatcher( nullptr );
  file_system_watcher_.reset();
}

void QmlPanelDisplay::requestReload()
{
  if ( reload_requested_ )
    return;
  reload_requested_ = true;
  QMetaObject::invokeMethod( this, [this] { reload(); }, Qt::QueuedConnection );
}

void QmlPanelDisplay::reload()
{
  reload_requested_ = false;
  // Reset the watches, the ones that are still needed are added again while loading.
  if ( file_system_watcher_ != nullptr )
    file_system_watcher_->removeAllWatches();
  file_tracker_->clear();

  destroyQuickView();

  // Recreating the engine resets all QML singletons and type registrations. The rviz context is
  // reused, so the properties registered from QML survive the reload.
  delete engine_;
  RvizQmlEngineConfig config;
  // No parent, a QQuickView observes but does not own an engine it is given.
  config.parent = nullptr;
  config.display_context = context_;
  config.rviz_context = rviz_context_;
  config.url_interceptor = file_tracker_.get();
  config.import_paths = import_paths_;
  config.plugin_paths = plugin_paths_;
  engine_ = createRvizQmlEngine( config );

  QString path = getPathToQml();
  if ( path.isEmpty() ) {
    deleteStatus( "Qml" );
    return;
  }
  QString resolved_path;
  try {
    resolved_path = resolvePath( path );
  } catch ( const std::exception &ex ) {
    // Resolving a package relative path throws if the package does not exist which is a typo away
    // for any user entered path.
    setStatus( StatusProperty::Error, "Qml",
               QString( "Could not resolve path: " ) + QString::fromStdString( ex.what() ) );
    return;
  }
  createQuickView( resolved_path );
}

void QmlPanelDisplay::createQuickView( const QString &resolved_path )
{
  // A QQuickView in a window container instead of a QQuickWidget because a QQuickWidget renders
  // with OpenGL in RViz's window: Qt then composites the whole window with OpenGL and leaves its
  // context current on the GUI thread, which breaks Ogre which expects its own context to stay
  // current. The view is a native window with its own context, so RViz's window is unaffected.
  quick_view_ = new QQuickView( engine_, nullptr );
  quick_view_->setResizeMode( QQuickView::SizeRootObjectToView );
  quick_view_->setColor( QApplication::palette().color( QPalette::Window ) );
  connect( quick_view_, &QQuickView::statusChanged, this, [this] { updateStatus(); } );
  gl_context_guard_->restoreContextAfterRenders( quick_view_ );
  view_container_ = QWidget::createWindowContainer( quick_view_, container_.get() );
  view_container_->setFocusPolicy( Qt::StrongFocus );
  container_->layout()->addWidget( view_container_ );
  // Loading a local file is synchronous, hence, statusChanged is emitted inside setSource.
  quick_view_->setSource( QUrl::fromLocalFile( resolved_path ) );
  static_cast<PanelContainer *>( container_.get() )->setView( quick_view_ );
  updateStatus();
}

void QmlPanelDisplay::destroyQuickView()
{
  if ( view_container_ == nullptr )
    return;
  static_cast<PanelContainer *>( container_.get() )->setView( nullptr );
  // Deleted directly and not using deleteLater because the engine the view references is deleted
  // right after. This is safe because a reload is always scheduled on the event loop.
  // The container owns the view.
  delete view_container_;
  view_container_ = nullptr;
  quick_view_ = nullptr;
  // Destroying the window on the GUI thread render loop leaves no context current at all.
  gl_context_guard_->restoreContext();
}

void QmlPanelDisplay::updateStatus()
{
  if ( quick_view_ == nullptr )
    return;
  switch ( quick_view_->status() ) {
  case QQuickView::Null:
    setStatus( StatusProperty::Warn, "Qml", "No qml file loaded." );
    break;
  case QQuickView::Loading:
    setStatus( StatusProperty::Warn, "Qml", "Loading." );
    break;
  case QQuickView::Ready:
    if ( quick_view_->rootObject() == nullptr ) {
      setStatus( StatusProperty::Error, "Qml", "Root object is not a QQuickItem." );
      break;
    }
    setStatus( StatusProperty::Ok, "Qml", "Successfully loaded." );
    break;
  case QQuickView::Error: {
    QStringList messages;
    for ( const QQmlError &error : quick_view_->errors() )
      messages.append( error.toString() );
    setStatus( StatusProperty::Error, "Qml", messages.join( '\n' ) );
    break;
  }
  }
}

void QmlPanelDisplay::load( const rviz_common::Config &config )
{
  Display::load( config );
  QVariant variant = configToVariant( config.mapGetChild( "Qml Configuration" ) );
  QVariantMap map;
  if ( variant.isValid() && variant.type() == QVariant::Map )
    map = variant.toMap();
  rviz_context_->setConfig( map );
  rviz_context_->load( config.mapGetChild( "Configuration" ) );
}

void QmlPanelDisplay::save( rviz_common::Config config ) const
{
  Display::save( config );
  writeToConfig( config.mapMakeChild( "Qml Configuration" ), rviz_context_->config() );
}
} // namespace hector_rviz_overlay
