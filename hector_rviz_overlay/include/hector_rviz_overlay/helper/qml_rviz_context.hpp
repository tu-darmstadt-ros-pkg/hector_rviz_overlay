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

#ifndef HECTOR_RVIZ_OVERLAY_QML_RVIZ_WRAPPER_H
#define HECTOR_RVIZ_OVERLAY_QML_RVIZ_WRAPPER_H

#include <QObject>
#include <QVariantMap>

#include "hector_rviz_overlay/displays/overlay_display.hpp"
#include "hector_rviz_overlay/helper/qml_rviz_property.hpp"
#include "hector_rviz_overlay/helper/qml_tool_manager.hpp"

namespace rviz_common
{
class DisplayContext;
}

namespace hector_rviz_overlay
{
class Overlay;

class QmlRvizContext : public QObject
{
Q_OBJECT
  // @formatter:off
  Q_PROPERTY( QVariantMap config READ config WRITE setConfig NOTIFY configChanged )
  Q_PROPERTY( bool visible READ visible NOTIFY visibleChanged )
  Q_PROPERTY( bool isFullscreen READ isFullscreen WRITE setIsFullscreen NOTIFY isFullscreenChanged )
  Q_PROPERTY( QString fixedFrame READ fixedFrame WRITE setFixedFrame NOTIFY fixedFrameChanged )
  Q_PROPERTY( QObject* toolManager READ toolManager CONSTANT )
  // @formatter:on
public:
  explicit QmlRvizContext( rviz_common::DisplayContext *context, const Overlay *overlay, bool visible = false );

  ~QmlRvizContext() override;

  const QVariantMap &config() const;

  void setConfig( const QVariantMap &config );

  bool visible() const;

  void setVisible( bool value );

  bool isFullscreen() const;

  void setIsFullscreen( bool value );

  QString fixedFrame() const;

  void setFixedFrame( const QString &frame );

  void load( const rviz_common::Config &config );

  void setConfigurationPropertyParent( rviz_common::properties::Property *parent );

  QObject *toolManager() const;

  /*!
   * Creates a position tracker with a notifying read-only position vector3d property, that will return the screen
   * coordinates of the given 3D point in the Ogre 3D View as x and y, and the distance of that point to the camera as
   * the z-coordinate.
   * @param x The x-value of the tracked point in the Ogre world.
   * @param y The y-value of the tracked point in the Ogre world.
   * @param z The z-value of the tracked point in the Ogre world.
   * @return An hector_rviz_overlay::positioning::OgrePositionTracker * mapping the given 3D point to its position on
   *   the screen.
   */
  Q_INVOKABLE QObject *createPositionTracker( double x, double y, double z );


  /*!
   * Registers a property container that can be used to organize properties in a tree structure.
   * @param name The name of the container.
   * @param description The description of the container.
   * @return The container to be used with the register overloads taking a parent property.
   */
  Q_INVOKABLE QObject *registerPropertyContainer( const QString &name, const QString &description = "" );

  Q_INVOKABLE QObject *registerPropertyContainer( QmlRvizProperty *parent, const QString &name,
                                                  const QString &description = "" );

  /*!
   * Registers a bool property on the display. See QmlRvizProperty
   * @param path The path of the property.
   * @param default_value The default value.
   * @param description A description of the property.
   * @return A wrapper of the registered property.
   */
  Q_INVOKABLE QObject *registerBoolProperty( const QString &name, bool default_value = false,
                                             const QString &description = "" );

  Q_INVOKABLE QObject *registerBoolProperty( QmlRvizProperty *parent, const QString &name, bool default_value = false,
                                             const QString &description = "" );

  /*!
   * Registers a float property on the display. See QmlRvizProperty
   * @param path The path of the property.
   * @param default_value The default value.
   * @param description A description of the property.
   * @return A wrapper of the registered property.
   */
  Q_INVOKABLE QObject *registerFloatProperty( const QString &name, float default_value = 0,
                                             const QString &description = "" );

  Q_INVOKABLE QObject *registerFloatProperty( QmlRvizProperty *parent, const QString &name, float default_value = 0,
                                             const QString &description = "" );

  /*!
   * Registers a topic property on the display. See QmlRvizProperty
   * @param path The path of the property.
   * @param default_value The default value.
   * @param message_type The type of the message, e.g., std_msgs/Int32.
   * @param description A description of the property.
   * @return A wrapper of the registered property.
   */
  Q_INVOKABLE QObject *registerRosTopicProperty( const QString &name, const QString &default_value = "",
                                                 const QString &message_type = "", const QString &description = "" );

  Q_INVOKABLE QObject *registerRosTopicProperty( QmlRvizProperty *parent, const QString &name,
                                                 const QString &default_value = "", const QString &message_type = "",
                                                 const QString &description = "" );

  /*!
   * Registers a string property on the display. See QmlRvizProperty
   * @param path The path of the property.
   * @param default_value The default value.
   * @param description A description of the property.
   * @return A wrapper of the registered property.
   */
  Q_INVOKABLE QObject *registerStringProperty( const QString &name, const QString &default_value = "",
                                               const QString &description = "" );

  Q_INVOKABLE QObject *registerStringProperty( QmlRvizProperty *parent, const QString &name,
                                               const QString &default_value = "", const QString &description = "" );

  /*!
   * Registers a tf2 frame property on the display. See QmlRvizProperty
   * @param path The path of the property.
   * @param default_value The default value.
   * @param description A description of the property.
   * @param include_fixed_frame Whether or not to include the fixed frame in the options. (Default: False)
   * @return A wrapper of the registered property.
   */
  Q_INVOKABLE QObject *registerTfFrameProperty( const QString &name, const QString &default_value = "",
                                                const QString &description = "", bool include_fixed_frame = false );

  Q_INVOKABLE QObject *registerTfFrameProperty( QmlRvizProperty *parent, const QString &name,
                                                const QString &default_value = "", const QString &description = "",
                                                bool include_fixed_frame = false );

signals:

  void configChanged();

  void visibleChanged();

  void isFullscreenChanged();

  void fixedFrameChanged();

private slots:
  void onWindowStateChanged( Qt::WindowState state );

private:
  void loadPropertyFromConfig( rviz_common::properties::Property *property );

  QVariantMap config_;
  rviz_common::Config property_config_;
  Qt::WindowState window_state_;
  rviz_common::DisplayContext *context_;
  std::unique_ptr<QmlToolManager> tool_manager_;
  const Overlay *overlay_;
  rviz_common::properties::Property *configuration_property_;
  bool visible_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_QML_RVIZ_WRAPPER_H
