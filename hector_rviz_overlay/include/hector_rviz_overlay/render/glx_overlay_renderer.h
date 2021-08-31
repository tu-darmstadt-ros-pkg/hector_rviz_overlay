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

#ifndef HECTOR_RVIZ_OVERLAY_GLX_OVERLAY_RENDERER_H
#define HECTOR_RVIZ_OVERLAY_GLX_OVERLAY_RENDERER_H

#include "overlay_renderer.h"

namespace Ogre
{
class Pass;
class SceneManager;
}

namespace rviz
{
class DisplayContext;
}

namespace hector_rviz_overlay
{

class QOpenGLWrapper;

/*!
 * @class GLXOverlayRenderer
 * @brief This class renders the overlays using OpenGL.
 *
 * This renderer supports both QmlOverlays and QWidgetOverlays.
 * The overlays are rendered into an FBO and drawn directly on top of the scene.
 */
class GLXOverlayRenderer : public OverlayRenderer
{
Q_OBJECT
private:
  class RenderTargetListener;
public:
  explicit GLXOverlayRenderer(rviz::DisplayContext *render_panel);

  ~GLXOverlayRenderer() override;

  void initialize() override;

  /*!
   * Called on first render.
   * Subclasses have to call the base implementation.
   */
  virtual void initializeOpenGL();

  void releaseResources() override;

  QPaintDevice *paintDevice() noexcept override;

  QOpenGLFramebufferObject *framebufferObject() override;

  bool framebufferObjectUpdated() noexcept override;

  QOpenGLContext *context() override;

protected:

  void prepareOverlay( OverlayPtr &overlay ) override;

  void releaseOverlay( OverlayPtr &overlay ) override;

  void hide() override;

  void redrawLastFrame() override;

  void prepareRender( int width, int height ) override;

  void finishRender() override;

  QOpenGLWrapper *qopengl_wrapper_ = nullptr;
  unsigned int shader_program_ = 0;
  unsigned int vertex_array_object_ = 0;
  unsigned int vertex_buffer_object_ = 0;
  unsigned int texture_ = 0;
  unsigned char *pixel_data_ = nullptr;
  bool can_share_texture_ = true;
  bool checked_can_share_texture_ = false;
  bool framebuffer_object_updated_ = true;

  Ogre::SceneManager *scene_manager_ = nullptr;
  Ogre::Pass *clear_pass_ = nullptr;

private:
  RenderTargetListener *render_target_listener_ = nullptr;
};
}

#endif //HECTOR_RVIZ_OVERLAY_GLX_OVERLAY_RENDERER_H
