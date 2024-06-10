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

#ifndef HECTOR_RVIZ_OVERLAY_IMAGE_OVERLAY_RENDERER_H
#define HECTOR_RVIZ_OVERLAY_IMAGE_OVERLAY_RENDERER_H

#include "hector_rviz_overlay/render/texture_overlay_renderer.hpp"

#include <QImage>

#include <OgreHardwarePixelBuffer.h>

namespace hector_rviz_overlay
{

/*!
 * @class QImageTextureOverlayRenderer
 * @brief This class renders QWidgetOverlay's into a QImage.
 *
 * The texture's memory is wrapped by a QImage that is used as a paint device for the QWidgetOverlays.
 * This renderer does not support the rendering of QmlOverlays!
 */
class QImageTextureOverlayRenderer : public TextureOverlayRenderer
{
Q_OBJECT
public:
  explicit QImageTextureOverlayRenderer( rviz_common::DisplayContext *context );

  void initialize() override;

  void releaseResources() override;

  QPaintDevice *paintDevice() noexcept override;

  QOpenGLFramebufferObject *framebufferObject() override;

  bool framebufferObjectUpdated() noexcept override;

  QOpenGLContext *context() override;

protected:
  void prepareRender(int width, int height) override;

  void finishRender() override;

  Ogre::HardwarePixelBufferSharedPtr buffer_;
  QImage paint_device_image_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_IMAGE_OVERLAY_RENDERER_H
