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
#include <rviz_common/render_panel.hpp>
#include <rviz_common/view_manager.hpp>

#include <OgreCamera.h>

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
  // The tracker is owned by QML but back-references the overlay; if the overlay is destroyed first,
  // detach from the camera and stop dereferencing it to avoid a use-after-free.
  if ( overlay_ != nullptr ) {
    connect( overlay_, &QObject::destroyed, this, [this]() {
      if ( camera_ != nullptr ) {
        camera_->removeListener( listener_.get() );
        camera_ = nullptr;
      }
      overlay_ = nullptr;
    } );
  }
  updateCamera();
}

OgrePositionTracker::~OgrePositionTracker()
{
  if ( camera_ != nullptr ) {
    camera_->removeListener( listener_.get() );
  }
}

QVector3D OgrePositionTracker::point() const
{
  return QVector3D( point_.x, point_.y, point_.z );
}

void OgrePositionTracker::setPoint( const QVector3D &point )
{
  Ogre::Vector3 value( point.x(), point.y(), point.z() );
  if ( value == point_ )
    return;
  point_ = value;
  emit pointChanged( point );
  checkPosition();
}

void OgrePositionTracker::checkPosition()
{
  // The geometry is empty until the overlay was rendered for the first time and is not updated
  // while it is hidden, so it can not be used to scale the position yet.
  if ( camera_ == nullptr || ( overlay_ != nullptr && overlay_->geometry().isEmpty() ) ) {
    updateVisible( false );
    return;
  }
  Ogre::Vector4 screen_point = camera_->getProjectionMatrix() * camera_->getViewMatrix() *
                               Ogre::Vector4( point_.x, point_.y, point_.z, 1 );
  // Clip space: the point is inside the view volume if |x|, |y|, |z| <= w. For a perspective
  // projection w is the view-space depth which is <= 0 at and behind the camera, for an
  // orthographic projection w is always 1 and only the depth range catches near and far plane.
  if ( screen_point.w <= 0 ) {
    // Dividing by a non-positive w would yield a mirrored or infinite position, keep the last one.
    updateVisible( false );
    return;
  }
  updateVisible( std::abs( screen_point.x ) <= screen_point.w &&
                 std::abs( screen_point.y ) <= screen_point.w &&
                 std::abs( screen_point.z ) <= screen_point.w );
  float x = screen_point.x * 0.5 / screen_point.w + 0.5;
  float y = -screen_point.y * 0.5 / screen_point.w + 0.5;
  float z = camera_->getProjectionType() == Ogre::PT_ORTHOGRAPHIC
                ? std::numeric_limits<float>::quiet_NaN()
                : screen_point.w;
  if ( overlay_ != nullptr ) {
    x *= overlay_->geometry().width() / overlay_->scale();
    y *= overlay_->geometry().height() / overlay_->scale();
  } else {
    // Without an overlay the coordinates are in logical pixels of rviz's 3D render panel.
    const QWidget *panel = context_->getViewManager()->getRenderPanel();
    x *= panel->width();
    y *= panel->height();
  }
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
  camera_ = nullptr;
  if ( auto *view_controller = context_->getViewManager()->getCurrent();
       view_controller != nullptr )
    camera_ = view_controller->getCamera();
  if ( camera_ != nullptr )
    camera_->addListener( listener_.get() );
}
} // namespace positioning
} // namespace hector_rviz_overlay
