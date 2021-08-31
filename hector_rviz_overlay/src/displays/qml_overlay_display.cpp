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

#include "hector_rviz_overlay/displays/qml_overlay_display.h"

#include "hector_rviz_overlay/helper/qml_rviz_context.h"

namespace hector_rviz_overlay
{


QmlOverlayDisplay::QmlOverlayDisplay() : QmlOverlayDisplay( false ) { }

QmlOverlayDisplay::QmlOverlayDisplay( bool allow_multiple ) : OverlayDisplay( allow_multiple )
{
  live_reload_property_ = new rviz::BoolProperty( "Live Reload", false,
                                                  "Watches the qml file and reloads it if it changed.\n"
                                                  " Useful for development.", this, SLOT( onLiveReloadChanged()));
}

void QmlOverlayDisplay::onOverlayStatusChanged( QmlOverlay::Status status )
{
  switch ( status )
  {
    case QmlOverlay::Ok:
      setStatus( rviz::StatusProperty::Ok, "Qml", "Successfully loaded." );
      break;
    case QmlOverlay::Uninitialized:
      setStatus( rviz::StatusProperty::Warn, "Qml", "Uninitialized." );
      break;
    case QmlOverlay::LoadingFailed:
      setStatus( rviz::StatusProperty::Error, "Qml", "Loading the qml file failed!" );
      break;
    case QmlOverlay::CreationFailed:
      setStatus( rviz::StatusProperty::Error, "Qml", "Creating the QML component failed!" );
      break;
    case QmlOverlay::Error:
      setStatus( rviz::StatusProperty::Error, "Qml", "Unknown error! Check the console for potential details." );
      break;
    default:
      setStatus( rviz::StatusProperty::Warn, "Qml", "Unknown status." );
  }
}

void QmlOverlayDisplay::onOverlayContextCreated()
{
  qml_overlay_->context()->setConfigurationPropertyParent( this );
  qml_overlay_->context()->load( overlay_config_ );
}

void QmlOverlayDisplay::onLiveReloadChanged()
{
  if ( qml_overlay_ == nullptr ) return;
  qml_overlay_->setLiveReloadEnabled( live_reload_property_->getBool());
}

QString QmlOverlayDisplay::getPathToQml()
{
  return QString();
}

UiOverlayPtr QmlOverlayDisplay::createOverlay()
{
  qml_overlay_ = std::make_shared<QmlOverlay>( metaObject()->className());
  connect( qml_overlay_.get(), &QmlOverlay::statusChanged, this, &QmlOverlayDisplay::onOverlayStatusChanged );
  connect( qml_overlay_.get(), &QmlOverlay::contextCreated, this, &QmlOverlayDisplay::onOverlayContextCreated );
  if ( qml_overlay_->context() != nullptr )
    onOverlayContextCreated();
  qml_overlay_->setLiveReloadEnabled( live_reload_property_->getBool());
  QString path = getPathToQml();
  if ( !path.isEmpty()) qml_overlay_->load( getPathToQml());
  return qml_overlay_;
}

namespace
{
QVariant configToQml( const rviz::Config &config )
{
  switch ( config.getType())
  {
    case rviz::Config::Map:
    {
      QVariantMap result;
      rviz::Config::MapIterator it = config.mapIterator();
      while ( it.isValid())
      {
        result.insert( it.currentKey(), configToQml( it.currentChild()));
        it.advance();
      }
      return result;
    }
    case rviz::Config::List:
    {
      QVariantList result;
      for ( int i = 0; i < config.listLength(); ++i )
      {
        result.append( configToQml( config.listChildAt( i )));
      }
      return result;
    }
    case rviz::Config::Value:
      return config.getValue();
    case rviz::Config::Empty:
    case rviz::Config::Invalid:
      return {};
  }
  return {};
}

void writeToConfig( rviz::Config config, const QVariantList &list );

void writeToConfig( rviz::Config config, const QVariant &variant )
{
  config.setValue( variant );
}

void writeToConfig( rviz::Config config, const QVariantMap &map )
{
  for ( auto &key : map.keys())
  {
    std::string std_key = key.toStdString();
    const QVariant &val = map[key];
    switch ( val.type())
    {
      case QVariant::Map:
        writeToConfig( config.mapMakeChild( key ), val.toMap());
        break;
      case QVariant::List:
        writeToConfig( config.mapMakeChild( key ), val.toList());
        break;
      default:
        writeToConfig( config.mapMakeChild( key ), val );
    }
  }
}

void writeToConfig( rviz::Config config, const QVariantList &list )
{
  for ( auto &item : list )
  {
    switch ( item.type())
    {
      case QVariant::Map:
        writeToConfig( config.listAppendNew(), item.toMap());
        break;
      case QVariant::List:
        writeToConfig( config.listAppendNew(), item.toList());
        break;
      default:
        writeToConfig( config.listAppendNew(), item );
    }
  }
}
}

void QmlOverlayDisplay::load( const rviz::Config &config )
{
  Display::load( config );
  overlay_config_ = config.mapGetChild( "Configuration" );
  QVariant variant = configToQml( config.mapGetChild( "Overlay Configuration" ));
  QVariantMap map;
  if ( variant.isValid() && variant.type() == QVariant::Map )
    map = variant.toMap();
  if ( qml_overlay_ == nullptr ) return;
  qml_overlay_->setConfiguration( map );
  if ( qml_overlay_->context() == nullptr ) return;
  qml_overlay_->context()->load( overlay_config_ );
}

void QmlOverlayDisplay::save( rviz::Config config ) const
{
  Display::save( config );
  writeToConfig( config.mapMakeChild( "Overlay Configuration" ), qml_overlay_->configuration());
}
}
