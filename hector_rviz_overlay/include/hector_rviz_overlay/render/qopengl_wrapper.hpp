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

#ifndef HECTOR_RVIZ_OVERLAY_QOPENGL_WRAPPER_H
#define HECTOR_RVIZ_OVERLAY_QOPENGL_WRAPPER_H

#include <QSize>

typedef struct _XDisplay Display;

class QOpenGLContext;
class QOpenGLPaintDevice;
class QOffscreenSurface;
class QOpenGLFramebufferObject;
class QPaintDevice;

namespace hector_rviz_overlay
{

class QOpenGLWrapper
{
private:
  struct NativeContextInformation;
public:
  explicit QOpenGLWrapper( Display *display, const QSize &size );

  ~QOpenGLWrapper();

  void makeCurrent();

  void doneCurrent();

  void prepareRender();

  void finishRender();

  const QSize &size() const;

  void setSize( const QSize &size );

  unsigned int texture() const;

  QPaintDevice *paintDevice() const noexcept;

  QOpenGLFramebufferObject *framebufferObject() const;

  QOpenGLContext *context() const;

private:
  void createFbo();

  QSize size_;

  QOpenGLContext *native_opengl_context_;
  QOpenGLContext *opengl_context_;
  QOffscreenSurface *surface_;
  QOpenGLFramebufferObject *fbo_, *texture_fbo_;
  QOpenGLPaintDevice *paint_device_;

  bool opengl_context_is_current_;

  NativeContextInformation *native_context_information_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_QOPENGL_WRAPPER_H
