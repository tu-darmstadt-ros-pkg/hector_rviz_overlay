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

#include "hector_rviz_overlay/popup/positioning/ogre_tracker.h"

#include <rviz/display_context.h>
#include <rviz/view_manager.h>

#include <OgreCamera.h>
#include <OgreVector4.h>

namespace hector_rviz_overlay
{

OgreTracker::OgreTracker( const Ogre::Vector3 &point, const rviz::DisplayContext *context )
  : point_( point ), context_( context )
{
}

QPoint OgreTracker::getPoint( int width, int height )
{
  Ogre::Camera *camera_ = context_->getViewManager()->getCurrent()->getCamera();
  Ogre::Vector4 screen_point =
    camera_->getProjectionMatrix() * camera_->getViewMatrix() * Ogre::Vector4( point_.x, point_.y, point_.z, 1 );
  double x = screen_point.x * 0.5 / screen_point.w + 0.5;
  double y = -screen_point.y * 0.5 / screen_point.w + 0.5;

  return { (int) (x * width), (int) (y * height) };
}
}
