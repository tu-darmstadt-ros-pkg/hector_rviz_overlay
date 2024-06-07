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

#ifndef HECTOR_RVIZ_OVERLAY_POSITION_TRACKER_H
#define HECTOR_RVIZ_OVERLAY_POSITION_TRACKER_H

#include <QObject>
#include <QVector3D>

namespace hector_rviz_overlay
{
namespace positioning
{
namespace anchor_points
{
enum AnchorPoint
{
  AnchorAuto,
  AnchorTopLeft,
  AnchorTopRight,
  AnchorCenter,
  AnchorBottomLeft,
  AnchorBottomRight
};
}
typedef anchor_points::AnchorPoint AnchorPoint;

/*!
 * Base class for point trackers which track a (potentially moving) point over time.
 */
class PositionTracker : public QObject
{
Q_OBJECT
  // @formatter:off
  //! The position of the tracked point on the screen. May have a z-component indicating the depth.
  //! If no depth is available, z will be NaN.
  Q_PROPERTY( QVector3D position READ position NOTIFY positionChanged );
  // @formatter:on
public:
  const QVector3D &position() const;

signals:

  void positionChanged( const QVector3D &position );

protected:
  void updatePosition( const QVector3D &position );

private:
  QVector3D position_;
};
}
}

#endif //HECTOR_RVIZ_OVERLAY_POSITION_TRACKER_H
