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

#include "hector_rviz_overlay/render/qimage_texture_overlay_renderer.hpp"

#include <OgreTexture.h>

namespace hector_rviz_overlay
{

QImageTextureOverlayRenderer::QImageTextureOverlayRenderer( rviz_common::DisplayContext *context )
    : TextureOverlayRenderer( context )
{
}

void QImageTextureOverlayRenderer::initialize() { /* Nothing to do */ }

void QImageTextureOverlayRenderer::releaseResources() { /* Nothing to do */ }

void QImageTextureOverlayRenderer::prepareRender( int width, int height )
{
  TextureOverlayRenderer::prepareRender( width, height );
  buffer_ = texture_->getBuffer();
  buffer_->lock( Ogre::HardwareBuffer::HBL_DISCARD );
  const Ogre::PixelBox &pixel_box = buffer_->getCurrentLock();

  auto data = static_cast<Ogre::uint8 *>( pixel_box.data );
  int bytes_per_line = pixel_box.getWidth() * 4;
  // Empties the texture by overwriting all ARGB data with 0.
  Ogre::uint8 *offset = data;
  for ( int i = 0; i < height; ++i ) {
    memset( offset, 0, width * 4 );
    offset += bytes_per_line;
  }
  paint_device_image_ =
      QImage( data, width, height, bytes_per_line, QImage::Format_ARGB32_Premultiplied );
}

void QImageTextureOverlayRenderer::finishRender()
{
  buffer_->unlock();
  buffer_.reset();
  TextureOverlayRenderer::finishRender();
}

QPaintDevice *QImageTextureOverlayRenderer::paintDevice() noexcept { return &paint_device_image_; }

QOpenGLFramebufferObject *QImageTextureOverlayRenderer::framebufferObject()
{
  throw std::logic_error(
      "ImageOverlayRenderer doesn't have a QOpenGLFramebufferObject."
      " Perhaps you've tried adding a QML overlay with a version of hector_rviz_overlay that was"
      " not compiled with the 'RENDER_OVERLAYS_USING_OPENGL' flag." );
}

QOpenGLContext *QImageTextureOverlayRenderer::context()
{
  throw std::logic_error(
      "ImageOverlayRenderer doesn't have a QOpenGLContext."
      " Perhaps you've tried adding a QML overlay with a version of hector_rviz_overlay that was"
      " not compiled with the 'RENDER_OVERLAYS_USING_OPENGL' flag." );
}

bool QImageTextureOverlayRenderer::framebufferObjectUpdated() noexcept { return false; }
} // namespace hector_rviz_overlay
