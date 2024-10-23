//
// Created by stefan on 10.06.24.
//

#include "hector_rviz_overlay/render/qopengl_texture_overlay_renderer.hpp"

#include "qopengl_wrapper.hpp"

#include <rviz_common/display_context.hpp>
#include <rviz_rendering/render_system.hpp>

namespace hector_rviz_overlay
{
QOpenGLTextureOverlayRenderer::QOpenGLTextureOverlayRenderer( rviz_common::DisplayContext *context )
    : TextureOverlayRenderer( context )
{
}

QOpenGLTextureOverlayRenderer::~QOpenGLTextureOverlayRenderer() = default;

void QOpenGLTextureOverlayRenderer::initialize()
{
  if ( qopengl_wrapper_ != nullptr )
    return;
  int gl_version = rviz_rendering::RenderSystem::get()->getGlVersion();
  qopengl_wrapper_ = std::make_unique<QOpenGLWrapper>( nullptr, geometry_.size(), gl_version );
  framebuffer_object_updated_ = true;
}

void QOpenGLTextureOverlayRenderer::prepareOverlay( OverlayPtr &overlay )
{
  qopengl_wrapper_->makeCurrent();
  TextureOverlayRenderer::prepareOverlay( overlay );
  qopengl_wrapper_->doneCurrent();
}

void QOpenGLTextureOverlayRenderer::releaseOverlay( OverlayPtr &overlay )
{
  qopengl_wrapper_->makeCurrent();
  TextureOverlayRenderer::releaseOverlay( overlay );
  qopengl_wrapper_->doneCurrent();
}

void QOpenGLTextureOverlayRenderer::prepareRender( int width, int height )
{
  TextureOverlayRenderer::prepareRender( width, height );
  qopengl_wrapper_->makeCurrent();
  if ( width != qopengl_wrapper_->size().width() || height != qopengl_wrapper_->size().height() ) {
    qopengl_wrapper_->setSize( QSize( width, height ) );
    framebuffer_object_updated_ = true;
    pixel_data_.resize( width * height * 4 );
  }
  qopengl_wrapper_->prepareRender();
}

void QOpenGLTextureOverlayRenderer::finishRender()
{
  framebuffer_object_updated_ = false;
  qopengl_wrapper_->finishRender();
  glBindTexture( GL_TEXTURE_2D, qopengl_wrapper_->texture() );
  glGetTexImage( GL_TEXTURE_2D, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data_.data() );
  qopengl_wrapper_->doneCurrent();
  glBindTexture( GL_TEXTURE_2D, texture_id_ );
  glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, qopengl_wrapper_->size().width(),
                qopengl_wrapper_->size().height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, pixel_data_.data() );
  TextureOverlayRenderer::finishRender();
}

void QOpenGLTextureOverlayRenderer::releaseResources()
{
  if ( qopengl_wrapper_ == nullptr )
    return;

  qopengl_wrapper_->makeCurrent();
  for ( auto &overlay : overlays_ ) { overlay->releaseRenderResources(); }
  overlays_.clear();
  qopengl_wrapper_->doneCurrent();
  qopengl_wrapper_.reset();
}

void QOpenGLTextureOverlayRenderer::updateTexture( unsigned texture_width, unsigned texture_height )
{
  TextureOverlayRenderer::updateTexture( texture_width, texture_height );
  texture_unit_state_->setTextureScale( 1, -1 );
}

QPaintDevice *QOpenGLTextureOverlayRenderer::paintDevice() noexcept
{
  return qopengl_wrapper_->paintDevice();
}

QOpenGLFramebufferObject *QOpenGLTextureOverlayRenderer::framebufferObject()
{
  return qopengl_wrapper_->framebufferObject();
}

bool QOpenGLTextureOverlayRenderer::framebufferObjectUpdated() noexcept
{
  return framebuffer_object_updated_;
}

QOpenGLContext *QOpenGLTextureOverlayRenderer::context() { return qopengl_wrapper_->context(); }
} // namespace hector_rviz_overlay
