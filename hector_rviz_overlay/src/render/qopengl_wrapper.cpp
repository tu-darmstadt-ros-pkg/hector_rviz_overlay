/*
 * Copyright (C) 2019  Stefan Fabian
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

#include "qopengl_wrapper.hpp"

#include "../logging.hpp"

#include <QOffscreenSurface>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLPaintDevice>
#include <QPainter>

#include <GL/glx.h>

namespace hector_rviz_overlay
{

struct QOpenGLWrapper::NativeContextInformation {
  GLXDrawable drawable;
  GLXContext context;
  Display *display;
};

QOpenGLWrapper::QOpenGLWrapper( QOpenGLContext *native_context, const QSize &size, int gl_version )
    : size_( size ), native_opengl_context_( native_context ), opengl_context_( nullptr ),
      surface_( nullptr ), fbo_( nullptr ), texture_fbo_( nullptr ), paint_device_( nullptr ),
      opengl_context_is_current_( false ), native_context_information_( nullptr )
{
  native_context_information_ = new NativeContextInformation;

  QSurfaceFormat format;
  format.setDepthBufferSize( 16 );
  format.setStencilBufferSize( 8 );
  format.setRenderableType( QSurfaceFormat::OpenGL );
  if ( gl_version != 0 )
    format.setVersion( gl_version / 100, gl_version % 100 );

  opengl_context_ = new QOpenGLContext;
  if ( native_opengl_context_ != nullptr )
    opengl_context_->setShareContext( native_opengl_context_ );
  opengl_context_->setFormat( format );

  if ( !opengl_context_->create() ) {
    LOG_ERROR( "OverlayManager: Fatal! Failed to create context!" );
  }
  LOG_INFO( "Hector RViz Overlay rendering using OpenGL %d.%d",
            opengl_context_->format().majorVersion(), opengl_context_->format().minorVersion() );
  surface_ = new QOffscreenSurface;
  surface_->setFormat( format );
  surface_->create();

  makeCurrent();

  paint_device_ = new QOpenGLPaintDevice( size_ );
  if ( size_.width() == 0 || size_.height() == 0 ) {
    doneCurrent();
    return;
  }
  createFbo();
  doneCurrent();
}

QOpenGLWrapper::~QOpenGLWrapper()
{
  if ( opengl_context_ != nullptr ) {
    makeCurrent();
    delete paint_device_;
    delete surface_;
    delete fbo_;
    delete texture_fbo_;
    doneCurrent();
    delete opengl_context_;
    delete native_opengl_context_;
    opengl_context_ = nullptr;
  }
  delete native_context_information_;
}

void QOpenGLWrapper::makeCurrent()
{
  if ( opengl_context_is_current_ )
    return;
  opengl_context_is_current_ = true;
  native_context_information_->display = glXGetCurrentDisplay();
  native_context_information_->drawable = glXGetCurrentDrawable();
  native_context_information_->context = glXGetCurrentContext();
  opengl_context_->makeCurrent( surface_ );
}

void QOpenGLWrapper::doneCurrent()
{
  if ( !opengl_context_is_current_ )
    return;
  opengl_context_is_current_ = false;
  opengl_context_->doneCurrent();
  glXMakeCurrent( native_context_information_->display, native_context_information_->drawable,
                  native_context_information_->context );
}

void QOpenGLWrapper::prepareRender()
{
  if ( paint_device_->size() != size() ) {
    paint_device_->setSize( size() );
    createFbo();
  }
  fbo_->bind();
  opengl_context_->functions()->glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void QOpenGLWrapper::finishRender()
{
  fbo_->release();
  opengl_context_->functions()->glFinish();
  QOpenGLFramebufferObject::blitFramebuffer( texture_fbo_, fbo_ );
}

const QSize &QOpenGLWrapper::size() const { return size_; }

void QOpenGLWrapper::setSize( const QSize &size ) { size_ = size; }

unsigned int QOpenGLWrapper::texture() const { return texture_fbo_->texture(); }

QPaintDevice *QOpenGLWrapper::paintDevice() const noexcept { return paint_device_; }

QOpenGLFramebufferObject *QOpenGLWrapper::framebufferObject() const { return fbo_; }

QOpenGLContext *QOpenGLWrapper::context() const { return opengl_context_; }

void QOpenGLWrapper::createFbo()
{
  delete fbo_;
  delete texture_fbo_;

  QOpenGLFramebufferObjectFormat format;
  format.setSamples( 4 );
  format.setAttachment( QOpenGLFramebufferObject::CombinedDepthStencil );
  fbo_ = new QOpenGLFramebufferObject( size(), format );
  texture_fbo_ = new QOpenGLFramebufferObject( size() );
}
} // namespace hector_rviz_overlay
