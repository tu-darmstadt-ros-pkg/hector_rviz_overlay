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

#include "hector_rviz_overlay/render/texture_overlay_renderer.hpp"

#include "ogre_gl_context.hpp"

#include <rviz_common/display_context.hpp>
#include <rviz_common/render_panel.hpp>
#include <rviz_common/view_manager.hpp>
#include <rviz_rendering/render_window.hpp>

#include <Ogre.h>
#include <OgreRenderTargetListener.h>
#include <Overlay/OgreOverlay.h>
#include <Overlay/OgreOverlayContainer.h>
#include <Overlay/OgreOverlayManager.h>
#include <RenderSystems/GL/OgreGLTexture.h>

#include "../logging.hpp"

namespace hector_rviz_overlay
{

class TextureOverlayRenderer::RenderTargetListener : public Ogre::RenderTargetListener
{
public:
  explicit RenderTargetListener( TextureOverlayRenderer *renderer ) : renderer_( renderer ) { }

  void preRenderTargetUpdate( const Ogre::RenderTargetEvent &evt ) override
  {
    Ogre::RenderTargetListener::preRenderTargetUpdate( evt );
    // Ogre assumes its context stays current between frames and only rebinds it when the target's
    // context changed or when the frame buffer is cleared, both of which happen after this
    // callback. Anything that makes another context current on this thread and leaves it current,
    // e.g. this renderer's own offscreen context or Qt Quick when it renders on the gui thread,
    // would make rendering the overlays and uploading them into the Ogre texture use that context.
    makeRenderTargetContextCurrent( evt.source );
    renderer_->render();
  }

  // Show the overlay only while the main RViz render window's viewport is
  // being updated. This listener is registered on the main render window only,
  // so it does not fire for Camera Display render windows.
  void preViewportUpdate( const Ogre::RenderTargetViewportEvent & ) override
  {
    if ( renderer_->ogre_overlay_ != nullptr && renderer_->overlay_active_ )
      renderer_->ogre_overlay_->show();
  }

  void postViewportUpdate( const Ogre::RenderTargetViewportEvent & ) override
  {
    if ( renderer_->ogre_overlay_ != nullptr )
      renderer_->ogre_overlay_->hide();
  }

private:
  TextureOverlayRenderer *renderer_;
};

namespace
{

constexpr const char *kOverlayTextureName = "hector_rviz_overlay_OverlayTexture";

size_t bit_ceil( size_t value )
{
#if __cplusplus >= 202002L
  return std::bit_ceil( value );
#else
  if ( value == 0 )
    return 1;
  --value;
  value |= value >> 1;
  value |= value >> 2;
  value |= value >> 4;
  value |= value >> 8;
  value |= value >> 16;
  return value + 1;
#endif
}
} // namespace

TextureOverlayRenderer::TextureOverlayRenderer( rviz_common::DisplayContext *context )
    : OverlayRenderer( context )
{
  render_target_listener_ = std::make_unique<RenderTargetListener>( this );
  rviz_rendering::RenderWindowOgreAdapter::addListener( render_panel_->getRenderWindow(),
                                                        render_target_listener_.get() );
  QObject::connect( render_panel_, &rviz_common::RenderPanel::destroyed, this,
                    &TextureOverlayRenderer::onRenderPanelDestroyed );
}

TextureOverlayRenderer::~TextureOverlayRenderer()
{
  if ( render_panel_ != nullptr ) {
    rviz_rendering::RenderWindowOgreAdapter::removeListener( render_panel_->getRenderWindow(),
                                                             render_target_listener_.get() );
  }

  if ( ogre_overlay_ != nullptr ) {
    ogre_overlay_->hide();
    Ogre::OverlayManager &overlay_manager = Ogre::OverlayManager::getSingleton();
    overlay_manager.destroy( ogre_overlay_ );
    if ( overlay_panel_ != nullptr )
      overlay_manager.destroyOverlayElement( overlay_panel_ );
    ogre_overlay_ = nullptr;
    overlay_panel_ = nullptr;
  }
  if ( material_ != nullptr ) {
    material_->unload();
    Ogre::MaterialManager::getSingleton().remove( material_->getName() );
  }
  if ( texture_ != nullptr )
    Ogre::TextureManager::getSingleton().remove( texture_->getName() );
}

void TextureOverlayRenderer::onRenderPanelDestroyed()
{
  render_panel_ = nullptr;
  render_target_listener_.reset();
}

void TextureOverlayRenderer::redrawLastFrame()
{
  // Mark the overlay as active. Actual visibility per render target is
  // toggled by the RenderTargetListener on the main render window.
  overlay_active_ = true;
}

void TextureOverlayRenderer::prepareRender( int width, int height )
{
  setupOverlay();
  // Create or resize texture if necessary
  updateTexture( width, height );

  if ( last_width_ != width || last_height_ != height ) {
    last_width_ = width;
    last_height_ = height;
    if ( overlay_panel_ != nullptr && texture_ != nullptr ) {
      overlay_panel_->setDimensions( (Ogre::Real)texture_->getWidth() / width,
                                     (Ogre::Real)texture_->getHeight() / height );
    }
  }
}

void TextureOverlayRenderer::finishRender() { overlay_active_ = true; }

void TextureOverlayRenderer::hide() { overlay_active_ = false; }

void TextureOverlayRenderer::updateTexture( unsigned int texture_width, unsigned int texture_height )
{
  if ( texture_multiple_of_two_required_ ) {
    texture_width = static_cast<unsigned int>( bit_ceil( texture_width ) );
    texture_height = static_cast<unsigned int>( bit_ceil( texture_height ) );
  }
  if ( texture_ && texture_->getWidth() == texture_width && texture_->getHeight() == texture_height )
    return;
  if ( texture_ ) {
    Ogre::TextureManager::getSingleton().remove( texture_->getName() );

    material_->getTechnique( 0 )->getPass( 0 )->removeAllTextureUnitStates();
    texture_unit_state_ = nullptr;
  }
  try {
    LOG_DEBUG( "Creating overlay texture of size (%u, %u)", texture_width, texture_height );
    texture_ = Ogre::TextureManager::getSingleton().createManual(
        kOverlayTextureName, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D, texture_width, texture_height, 0, Ogre::PF_A8R8G8B8,
        Ogre::TextureUsage::TU_STATIC | Ogre::TextureUsage::TU_RENDERTARGET );
  } catch ( std::exception &ex ) {
    LOG_ERROR( "Caught exception while creating overlay texture: %s", ex.what() );
    // createManual registers the resource by name before initialising its GPU resources (e.g. the
    // render-target FBO). If that init throws, the name stays registered but no usable texture is
    // returned, so later create attempts would fail with "texture already exists". Drop the
    // half-created resource so it can be recreated.
    texture_.reset();
    auto &texture_manager = Ogre::TextureManager::getSingleton();
    if ( texture_manager.resourceExists( kOverlayTextureName ) )
      texture_manager.remove( kOverlayTextureName );
  }
  if ( !texture_ ) {
    if ( texture_multiple_of_two_required_ ) {
      LOG_WARN( "Failed to create texture of size (%u, %u)!", texture_width, texture_height );
    } else {
      texture_multiple_of_two_required_ = true;
      LOG_WARN(
          "Failed to create texture of size (%u, %u)! Switching to textures sized by powers of 2.",
          texture_width, texture_height );
      updateTexture( texture_width, texture_height );
    }
    return;
  }
  if ( auto *gl_texture = dynamic_cast<const Ogre::GLTexture *>( texture_.get() );
       gl_texture != nullptr ) {
    texture_id_ = gl_texture->getGLID();
  }

  texture_unit_state_ =
      material_->getTechnique( 0 )->getPass( 0 )->createTextureUnitState( texture_->getName() );
}

void TextureOverlayRenderer::setupOverlay()
{
  if ( ogre_overlay_ != nullptr )
    return;
  Ogre::OverlayManager &overlay_manager = Ogre::OverlayManager::getSingleton();
  ogre_overlay_ = overlay_manager.create( "hector_rviz_overlay/Overlay" );
  material_ = Ogre::MaterialManager::getSingleton().create(
      "hector_rviz_overlay/OverlayMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
  auto material_pass = material_->getTechnique( 0 )->getPass( 0 );
  material_pass->setSceneBlending( Ogre::SBT_TRANSPARENT_ALPHA );
  material_pass->setDepthCheckEnabled( false );
  material_pass->setDepthWriteEnabled( false );
  material_pass->setLightingEnabled( false );

  overlay_panel_ = dynamic_cast<Ogre::OverlayContainer *>(
      overlay_manager.createOverlayElement( "Panel", "hector_rviz_overlay/MainPanel" ) );
  overlay_panel_->setPosition( 0.0, 0.0 );
  overlay_panel_->setDimensions( 1.0, 1.0 );
  overlay_panel_->setMaterial( material_ );

  ogre_overlay_->add2D( overlay_panel_ );
  // Default to hidden; the RenderTargetListener will show it during the main
  // render window's viewport update only.
  ogre_overlay_->hide();
}
} // namespace hector_rviz_overlay
