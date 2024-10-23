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

#include "hector_rviz_overlay/positioning/ogre_position_tracker.hpp"
#include "hector_rviz_overlay/overlay.hpp"

#include <rviz_common/display_context.hpp>
#include <rviz_common/view_manager.hpp>

#include <OgreCamera.h>
#include <OgreViewport.h>

namespace hector_rviz_overlay
{
namespace positioning
{

class OgrePositionTracker::Listener : public Ogre::Camera::Listener
{
public:
  explicit Listener( OgrePositionTracker *parent ) : parent_( parent ) { }

  void cameraPreRenderScene( Ogre::Camera * ) override { parent_->checkPosition(); }

  void cameraDestroyed( Ogre::Camera * ) override { parent_->updateCamera(); }

private:
  OgrePositionTracker *parent_;
};

OgrePositionTracker::OgrePositionTracker( const Ogre::Vector3 &point,
                                          const rviz_common::DisplayContext *context,
                                          const Overlay *overlay )
    : point_( point ), context_( context ), overlay_( overlay )
{
  listener_.reset( new OgrePositionTracker::Listener( this ) );
  updateCamera();
}

OgrePositionTracker::~OgrePositionTracker()
{
  if ( camera_ != nullptr ) {
    camera_->removeListener( listener_.get() );
  }
}

void OgrePositionTracker::checkPosition()
{
  Ogre::Vector4 screen_point = camera_->getProjectionMatrix() * camera_->getViewMatrix() *
                               Ogre::Vector4( point_.x, point_.y, point_.z, 1 );
  float x = screen_point.x * 0.5 / screen_point.w + 0.5;
  float y = -screen_point.y * 0.5 / screen_point.w + 0.5;
  float z = screen_point.z < 0 ? std::numeric_limits<float>::quiet_NaN() : screen_point.z;
  const Ogre::Viewport *viewport = camera_->getViewport();
  x *= viewport->getActualWidth() / overlay_->scale();
  y *= viewport->getActualHeight() / overlay_->scale();
  if ( std::abs( x - position().x() ) < 1E-2 && std::abs( y - position().y() ) < 1E-2 &&
       ( ( std::isnan( z ) && std::isnan( position().z() ) ) ||
         ( !std::isnan( z ) && !std::isnan( position().z() ) &&
           std::abs( z - position().z() ) < 1E-2 ) ) )
    return;

  updatePosition( QVector3D( x, y, z ) );
}

void OgrePositionTracker::updateCamera()
{
  if ( camera_ != nullptr )
    camera_->removeListener( listener_.get() );
  camera_ = context_->getViewManager()->getCurrent()->getCamera();
  if ( camera_ != nullptr )
    camera_->addListener( listener_.get() );
}
} // namespace positioning
} // namespace hector_rviz_overlay
