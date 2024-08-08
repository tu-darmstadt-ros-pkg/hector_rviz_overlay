/*
 * Copyright (C) 2018  Stefan Fabian
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

#ifndef HECTOR_RVIZ_OVERLAY_OGRE_TRACKER_H
#define HECTOR_RVIZ_OVERLAY_OGRE_TRACKER_H

#include "hector_rviz_overlay/popup/positioning/point_tracker.hpp"

#include <Ogre.h>

namespace rviz_common
{
class DisplayContext;
}

namespace hector_rviz_overlay
{

class OgreTracker : public PointTracker
{
public:
  OgreTracker( const Ogre::Vector3 &point, const rviz_common::DisplayContext *context );

  QPoint getPoint( int width, int height ) override;

protected:
  Ogre::Vector3 point_;
  const rviz_common::DisplayContext *context_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_OGRE_TRACKER_H
