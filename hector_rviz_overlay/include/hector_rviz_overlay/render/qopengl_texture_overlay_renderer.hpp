//
// Created by stefan on 10.06.24.
//

#ifndef QOPENGL_TEXTURE_OVERLAY_RENDERER_HPP
#define QOPENGL_TEXTURE_OVERLAY_RENDERER_HPP

#include "hector_rviz_overlay/render/texture_overlay_renderer.hpp"

namespace hector_rviz_overlay
{
class QOpenGLWrapper;

class QOpenGLTextureOverlayRenderer : public TextureOverlayRenderer
{
public:
  explicit QOpenGLTextureOverlayRenderer( rviz_common::DisplayContext *context );

  ~QOpenGLTextureOverlayRenderer() override;

  void releaseResources() override;

  QPaintDevice *paintDevice() noexcept override;
  QOpenGLFramebufferObject *framebufferObject() override;
  bool framebufferObjectUpdated() noexcept override;
  QOpenGLContext *context() override;

protected:
  void updateTexture( unsigned texture_width, unsigned texture_height ) override;

  void initialize() override;

  void prepareOverlay( OverlayPtr &overlay ) override;

  void releaseOverlay( OverlayPtr &overlay ) override;

  void prepareRender( int width, int height ) override;

  void finishRender() override;

  std::unique_ptr<QOpenGLWrapper> qopengl_wrapper_;
  std::vector<unsigned char> pixel_data_;
  bool framebuffer_object_updated_ = false;
};
} // namespace hector_rviz_overlay

#endif // QOPENGL_TEXTURE_OVERLAY_RENDERER_HPP
