/*
 * Copyright (C) 2017  Stefan Fabian
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

#ifndef HECTOR_RVIZ_OVERLAY_RENDERER_H
#define HECTOR_RVIZ_OVERLAY_RENDERER_H

class QOpenGLContext;
class QOpenGLFramebufferObject;
class QPaintDevice;
class QWindow;

namespace hector_rviz_overlay
{

/*!
 * @class Renderer
 * @brief The base class for all renderer's. Provides resources for the overlays to paint themselves.
 */
class Renderer
{
public:
  //! @return The paint device that can be used to render in this renderer.
  virtual QPaintDevice *paintDevice() noexcept = 0;

  /*!
   * @throws std::logic_error If the renderer doesn't have a QOpenGLFramebufferObject.
   * @return The QOpenGLFramebufferObject used by this renderer.
   */
  virtual QOpenGLFramebufferObject *framebufferObject() = 0;

  //! @return Returns true if the framebufferObject was updated since the last render call and should be updated.
  virtual bool framebufferObjectUpdated() noexcept = 0;

  /*!
   * @throws std::logic_error If the renderer doesn't have a QOpenGLContext
   * @return The QOpenGLContext used by this renderer.
   */
  virtual QOpenGLContext *context() = 0;

  virtual QWindow *window() = 0;
};
}

#endif //HECTOR_RVIZ_OVERLAY_RENDERER_H
