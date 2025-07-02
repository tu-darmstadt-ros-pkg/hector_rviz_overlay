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

#include <rviz_common/display_context.hpp>
#include <rviz_common/render_panel.hpp>
#include <rviz_common/view_manager.hpp>
#include <rviz_rendering/render_window.hpp>

#include <Ogre.h>
#include <OgreRectangle2D.h>
#include <OgreRenderTargetListener.h>
#include <OgreSceneNode.h>
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
    renderer_->render();
  }

private:
  TextureOverlayRenderer *renderer_;
};

namespace
{

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
  // Make sure it's visible, that's all
  rectangle_->setVisible( true );
}

void TextureOverlayRenderer::prepareRender( int width, int height )
{
  setupOverlay();
  // Create or resize texture if necessary
  updateTexture( width, height );

  if ( last_width_ != width || last_height_ != height ) {
    last_width_ = width;
    last_height_ = height;
  }
}

void TextureOverlayRenderer::finishRender() { rectangle_->setVisible( true ); }

void TextureOverlayRenderer::hide() { rectangle_->setVisible( false ); }

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
        "hector_rviz_overlay_OverlayTexture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D, texture_width, texture_height, 0, Ogre::PF_A8R8G8B8,
        Ogre::TextureUsage::TU_STATIC | Ogre::TextureUsage::TU_RENDERTARGET );
  } catch ( std::exception &ex ) {
    LOG_ERROR( "Caught exception while creating overlay texture: %s", ex.what() );
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
  if ( material_ != nullptr )
    return;
  material_ = Ogre::MaterialManager::getSingleton().create(
      "hector_rviz_overlay/OverlayMaterial", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME );
  auto material_pass = material_->getTechnique( 0 )->getPass( 0 );
  material_pass->setSceneBlending( Ogre::SBT_TRANSPARENT_ALPHA );
  material_pass->setDepthCheckEnabled( false );
  material_pass->setDepthWriteEnabled( false );
  material_pass->setLightingEnabled( false );

  rectangle_ = new Ogre::Rectangle2D( true );
  rectangle_->setUseIdentityProjection( true );
  rectangle_->setUseIdentityView( true );
  rectangle_->setBoundingBox(Ogre::AxisAlignedBox::BOX_INFINITE);
  rectangle_->setCorners( -1, 1, 1, -1 ); // Full screen rectangle
  rectangle_->setMaterial( material_ );
  rectangle_->setRenderQueueGroup( Ogre::RENDER_QUEUE_OVERLAY );
  context_->getSceneManager()->getRootSceneNode()->attachObject( rectangle_ );
}
} // namespace hector_rviz_overlay
