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

#include "hector_rviz_overlay/render/overlay_renderer.hpp"

#include <OgreMaterial.h>
#include <OgreTexture.h>

namespace Ogre
{
class Overlay;
class OverlayContainer;
} // namespace Ogre

namespace rviz_common
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
  explicit TextureOverlayRenderer( rviz_common::DisplayContext *context );

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
  void prepareRender( int width, int height ) override;

  /*!
   * @inherit
   * This is called after everything was rendered and after this method called it is expected that the final rendered
   * image is in the texture.
   * When overriding this method make sure to call the base implementation.
   */
  void finishRender() override;

  /*!
   * Creates a new texture with the given width and height if necessary. This method may destroy the old texture.
   * Automatically handles if texture have to be sized using powers of 2.
   * After this method was called the texture_, texture_unit_state and texture_id_ properties are initialized
   * (in case of an error it can be a nullptr).
   * If you need to make changes to the texture or texture unit, override this method and call the base implementation
   * before making your changes.
   * @param texture_width Width of the texture.
   * @param texture_height Height of the texture.
   */
  virtual void updateTexture( unsigned int texture_width, unsigned int texture_height );

  Ogre::Overlay *ogre_overlay_;
  Ogre::OverlayContainer *overlay_panel_;
  Ogre::MaterialPtr material_;
  Ogre::TexturePtr texture_;
  Ogre::TextureUnitState *texture_unit_state_;
  unsigned int texture_id_ = 0;

  rviz_common::RenderPanel *render_panel_;

private:
  class RenderTargetListener;

  RenderTargetListener *render_target_listener_;

  int last_width_ = 0;
  int last_height_ = 0;

  bool texture_multiple_of_two_required_ = false;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_TEXTURE_OVERLAY_RENDERER_H
