/*
 * Copyright (C) 2020  Stefan Fabian
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

#ifndef HECTOR_RVIZ_OVERLAY_OGRE_POSITION_TRACKER_H
#define HECTOR_RVIZ_OVERLAY_OGRE_POSITION_TRACKER_H

#include "hector_rviz_overlay/positioning/position_tracker.hpp"

#include <Ogre.h>

namespace Ogre
{
class Camera;
}

namespace rviz_common
{
class DisplayContext;
}

namespace hector_rviz_overlay
{
class Overlay;

namespace positioning
{

/*!
 * @brief Tracks a 3D point in the Ogre scene.
 *
 * Returns x and y position in overlay coordinates (pixel, already divided by the overlay's scale).
 * Returns the depth of the point along the camera's viewing direction as z-coordinate unless it is
 * not available, e.g., for an orthographic projection, in which case z is set to NaN.
 */
class OgrePositionTracker : public PositionTracker
{
  Q_OBJECT
  //! The tracked point in the Ogre world.
  Q_PROPERTY( QVector3D point READ point WRITE setPoint NOTIFY pointChanged )
public:
  OgrePositionTracker( const Ogre::Vector3 &point, const rviz_common::DisplayContext *context,
                       const Overlay *overlay );

  ~OgrePositionTracker() override;

  QVector3D point() const;

  //! Moves the tracked point to the given position in the Ogre world.
  void setPoint( const QVector3D &point );

signals:

  void pointChanged( const QVector3D &point );

private:
  void checkPosition();

  void updateCamera();

  Ogre::Vector3 point_;
  const rviz_common::DisplayContext *context_;
  const Overlay *overlay_;
  Ogre::Camera *camera_ = nullptr;

  class Listener;

  std::unique_ptr<Listener> listener_;
};
} // namespace positioning
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_OGRE_POSITION_TRACKER_H
