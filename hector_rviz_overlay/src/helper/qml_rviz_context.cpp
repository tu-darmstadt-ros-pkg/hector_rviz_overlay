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

#include "hector_rviz_overlay/helper/qml_rviz_context.h"
#include "hector_rviz_overlay/positioning/ogre_position_tracker.h"

#include <rviz/properties/bool_property.h>
#include <rviz/properties/float_property.h>
#include <rviz/properties/ros_topic_property.h>
#include <rviz/properties/string_property.h>
#include <rviz/properties/tf_frame_property.h>
#include <rviz/display_context.h>
#include <rviz/window_manager_interface.h>
#include <QWidget>
#include <QWindow>

namespace hector_rviz_overlay
{

QmlRvizContext::QmlRvizContext( rviz::DisplayContext *context, const Overlay *overlay, bool visible )
  : context_( context ), overlay_( overlay ), visible_( visible )
{
  configuration_property_ = new rviz::Property( "Configuration", QVariant(),
                                                "Container for configurable settings of the overlay." );
  tool_manager_ = new QmlToolManager( context_->getToolManager());
  // If rviz is used as widget inside an application, the window manager interface may not be available
  rviz::WindowManagerInterface *wmi = context_->getWindowManager();
  QWindow *window = wmi == nullptr ? nullptr : wmi->getParentWindow()->windowHandle();
  if ( window == nullptr )
  {
    window_state_ = Qt::WindowNoState;
    return;
  }
  connect( window, &QWindow::windowStateChanged, this, &QmlRvizContext::onWindowStateChanged );
  window_state_ = window->windowState();
}

QmlRvizContext::~QmlRvizContext() = default;

const QVariantMap &QmlRvizContext::config() const
{
  return config_;
}

void QmlRvizContext::setConfig( const QVariantMap &config )
{
  config_ = config;
  emit configChanged();
}

QObject *QmlRvizContext::createPositionTracker( double x, double y, double z )
{
  return new positioning::OgrePositionTracker( Ogre::Vector3( x, y, z ), context_, overlay_ );
}

bool QmlRvizContext::visible() const
{
  return visible_;
}

void QmlRvizContext::setVisible( bool value )
{
  visible_ = value;
  emit visibleChanged();
}

bool QmlRvizContext::isFullscreen() const
{
  return context_->getWindowManager()->getParentWindow()->isFullScreen();
}

void QmlRvizContext::setIsFullscreen( bool value )
{
  // If rviz is used as widget inside an application, the window manager interface may not be available
  rviz::WindowManagerInterface *wmi = context_->getWindowManager();
  if ( wmi == nullptr )
  {
    ROS_ERROR_NAMED( "hector_rviz_overlay", "setIsFullscreen failed: Window could not be obtained! "
                                            "Implement the WindowManagerInterface in the DisplayContext." );
    return;
  }
  // Unfortunately this method is not exposed in the public interface, hence, we need to work around this using Qt
  QMetaObject::invokeMethod( context_->getWindowManager()->getParentWindow(), "setFullScreen", Q_ARG( bool, value ));
}

void QmlRvizContext::onWindowStateChanged( Qt::WindowState state )
{
  if ((state & Qt::WindowFullScreen) != (window_state_ & Qt::WindowFullScreen)) emit isFullscreenChanged();
  window_state_ = state;
}

void QmlRvizContext::setConfigurationPropertyParent( rviz::Property *parent )
{
  if ( configuration_property_->getParent() != nullptr )
    configuration_property_->getParent()->takeChild( configuration_property_ );
  configuration_property_->setParent( parent );
  if ( parent != nullptr ) parent->addChild( configuration_property_ );
}

namespace
{
rviz::Property *findChildProperty( rviz::Property *property, const QString &path )
{
  for ( int i = 0; i < property->numChildren(); ++i )
  {
    if ( property->childAt( i )->getName() != path ) continue;
    return property->childAt( i );
  }
  return nullptr;
}

void loadConfig( const rviz::Config &config, rviz::Property *property )
{
  if ( !config.isValid()) return;
  if ( config.getType() == rviz::Config::Value )
  {
    property->setValue( config.getValue());
    return;
  }
  if ( config.getType() != rviz::Config::Map ) return;
  for ( int i = 0; i < property->numChildren(); ++i )
  {
    loadConfig( config.mapGetChild( property->childAt( i )->getName()), property->childAt( i ));
  }
}

QVariant getValue( const rviz::Config &config, std::vector<QString> &path )
{
  if ( path.empty())
  {
    if ( !config.isValid() || config.getType() != rviz::Config::Value ) return {};
    return config.getValue();
  }
  if ( !config.isValid() || config.getType() != rviz::Config::Map ) return {};
  const rviz::Config &child = config.mapGetChild( path.back());
  path.pop_back();
  return getValue( child, path );
}
}

void QmlRvizContext::loadPropertyFromConfig( rviz::Property *property )
{
  const rviz::Property *prop = property;
  std::vector<QString> path;
  while ( prop != configuration_property_ && prop != nullptr )
  {
    path.push_back( prop->getName());
    prop = prop->getParent();
  }
  QVariant value = getValue( property_config_, path );
  if ( !value.isValid()) return;
  property->setValue( value );
}

void QmlRvizContext::load( const rviz::Config &config )
{
  property_config_ = config;
  loadConfig( config, configuration_property_ );
}

QObject *QmlRvizContext::toolManager() const
{
  return tool_manager_;
}

QObject *QmlRvizContext::registerPropertyContainer( const QString &name, const QString &description )
{
  QmlRvizProperty tmp( configuration_property_ );
  return registerPropertyContainer( &tmp, name, description );
}

QObject *QmlRvizContext::registerPropertyContainer( QmlRvizProperty *parent, const QString &name,
                                                    const QString &description )
{
  auto *prop = findChildProperty( parent->property(), name );
  if ( prop == nullptr )
  {
    prop = new rviz::Property( name, QVariant(), description, parent->property());
    loadPropertyFromConfig( prop );
  }
  return new QmlRvizProperty( prop );
}

QObject *QmlRvizContext::registerBoolProperty( const QString &name, bool default_value, const QString &description )
{
  QmlRvizProperty tmp( configuration_property_ );
  return registerBoolProperty( &tmp, name, default_value, description );
}

QObject *QmlRvizContext::registerBoolProperty( QmlRvizProperty *parent, const QString &name, bool default_value,
                                               const QString &description )
{
  auto *prop = findChildProperty( parent->property(), name );
  if ( prop == nullptr )
  {
    prop = new rviz::BoolProperty( name, default_value, description, parent->property());
    loadPropertyFromConfig( prop );
  }
  return new QmlRvizProperty( prop );
}

QObject *QmlRvizContext::registerFloatProperty( const QString &name, float default_value,
                                                const QString &description )
{
  QmlRvizProperty tmp( configuration_property_ );
  return registerFloatProperty( &tmp, name, default_value, description );
}

QObject *QmlRvizContext::registerFloatProperty( QmlRvizProperty *parent, const QString &name, float default_value,
                                                const QString &description )
{
  auto *prop = findChildProperty( parent->property(), name );
  if ( prop == nullptr )
  {
    prop = new rviz::FloatProperty( name, default_value, description, parent->property());
    loadPropertyFromConfig( prop );
  }
  return new QmlRvizProperty( prop );
}

QObject *QmlRvizContext::registerStringProperty( const QString &name, const QString &default_value,
                                                 const QString &description )
{
  QmlRvizProperty tmp( configuration_property_ );
  return registerStringProperty( &tmp, name, default_value, description );
}

QObject *QmlRvizContext::registerStringProperty( QmlRvizProperty *parent, const QString &name,
                                                 const QString &default_value, const QString &description )
{
  auto *prop = findChildProperty( parent->property(), name );
  if ( prop == nullptr )
  {
    prop = new rviz::StringProperty( name, default_value, description, parent->property());
    loadPropertyFromConfig( prop );
  }
  return new QmlRvizProperty( prop );
}

QObject *QmlRvizContext::registerRosTopicProperty( const QString &name, const QString &default_value,
                                                   const QString &message_type, const QString &description )
{
  QmlRvizProperty tmp( configuration_property_ );
  return registerRosTopicProperty( &tmp, name, default_value, message_type, description );
}

QObject *QmlRvizContext::registerRosTopicProperty( QmlRvizProperty *parent, const QString &name,
                                                   const QString &default_value, const QString &message_type,
                                                   const QString &description )
{
  auto *prop = findChildProperty( parent->property(), name );
  if ( prop == nullptr )
  {
    prop = new rviz::RosTopicProperty( name, default_value, message_type, description, parent->property());
    loadPropertyFromConfig( prop );
  }
  return new QmlRvizProperty( prop );
}

QObject *QmlRvizContext::registerTfFrameProperty( const QString &name, const QString &default_value,
                                                  const QString &description, bool include_fixed_frame )
{

  QmlRvizProperty tmp( configuration_property_ );
  return registerTfFrameProperty( &tmp, name, default_value, description, include_fixed_frame );
}

QObject *QmlRvizContext::registerTfFrameProperty( QmlRvizProperty *parent, const QString &name,
                                                  const QString &default_value, const QString &description,
                                                  bool include_fixed_frame )
{
  auto *prop = findChildProperty( parent->property(), name );
  if ( prop == nullptr )
  {
    prop = new rviz::TfFrameProperty( name, default_value, description, parent->property(),
                                      context_->getFrameManager(), include_fixed_frame );
    loadPropertyFromConfig( prop );
  }
  return new QmlRvizProperty( prop );
}
}
