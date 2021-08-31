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

#ifndef HECTOR_RVIZ_OVERLAY_TEXTURE_OVERLAY_RENDERER_H
#define HECTOR_RVIZ_OVERLAY_TEXTURE_OVERLAY_RENDERER_H

#include "hector_rviz_overlay/render/overlay_renderer.h"

#include <OgreMaterial.h>
#include <OgreTexture.h>

namespace Ogre
{
class Overlay;
class OverlayContainer;
}

namespace rviz
{
class DisplayContext;
}

namespace hector_rviz_overlay
{

/*!
 * @class TextureOverlayRenderer
 * @brief An abstract base class for texture based renderers.
 *
 * Provides implementations to create and register an Ogre overlay using a texture that is drawn on top of the scene.
 * Also handles resizing the texture and makes sure the rendering is done after each frame.
 */
class TextureOverlayRenderer : public OverlayRenderer
{
public:
  explicit TextureOverlayRenderer( rviz::DisplayContext *context );

  ~TextureOverlayRenderer() override;

protected slots:

  void onRenderPanelDestroyed();

protected:

  void hide() override;

  void redrawLastFrame() override;

  /*!
   * @inherit
   * Makes sure that the texture has the appropriate size and resizes the overlay if necessary.
   * When overriding make sure to call the base implementation.
   */
  void prepareRender(int width, int height) override;

  /*!
   * @inherit
   * This is called after everything was rendered and after this method called it is expected that the final rendered
   * image is in the texture.
   * When overriding this method make sure to call the base implementation.
   */
  void finishRender() override;

  /*!
   * Creates a new texture with the given width and height if necessary.
   * Automatically handles if texture have to be sized using powers of 2.
   * After this method was called the texture_ property should be initialized (in case of an error it can be a nullptr)
   * and ready for rendering, e.g., by attaching the texture to the material_.
   * @param texture_width Width of the texture.
   * @param texture_height Height of the texture.
   */
  virtual void updateTexture( unsigned int texture_width, unsigned int texture_height );

  Ogre::Overlay *ogre_overlay_;
  Ogre::OverlayContainer *overlay_panel_;
  Ogre::MaterialPtr material_;
  Ogre::TexturePtr texture_;

  rviz::RenderPanel *render_panel_;

private:
  class RenderTargetListener;

  RenderTargetListener *render_target_listener_;

  int last_width_ = 0;
  int last_height_ = 0;

  bool texture_multiple_of_two_required_ = false;
};
} // hector_rviz_overlay

#endif //HECTOR_RVIZ_OVERLAY_TEXTURE_OVERLAY_RENDERER_H
