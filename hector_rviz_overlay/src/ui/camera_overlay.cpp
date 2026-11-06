/*
 * Copyright (C) 2026  Aljoscha Schmidt
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

#include "hector_rviz_overlay/ui/camera_overlay.hpp"
#include "hector_rviz_overlay/helper/qml_rviz_context.hpp"

#include "../logging.hpp"
#include "../render/ogre_gl_context.hpp"

#include <rviz_common/display_context.hpp>
#include <rviz_common/display_group.hpp>
#include <rviz_common/frame_manager_iface.hpp>
#include <rviz_common/render_panel.hpp>
#include <rviz_common/view_manager.hpp>
#include <rviz_rendering/render_window.hpp>

#include <Ogre.h>

#include <QQmlContext>
#include <QQuickWindow>
#include <QSGSimpleTextureNode>

namespace hector_rviz_overlay
{

class CameraOverlay::Listener : public Ogre::FrameListener, public Ogre::RenderTargetListener
{
public:
  explicit Listener( CameraOverlay *parent ) : parent_( parent ) { }

  bool frameStarted( const Ogre::FrameEvent & ) override
  {
    parent_->prepareFrame();
    return true;
  }

  void preRenderTargetUpdate( const Ogre::RenderTargetEvent & ) override { parent_->preRender(); }

  void postRenderTargetUpdate( const Ogre::RenderTargetEvent & ) override { parent_->postRender(); }

private:
  CameraOverlay *parent_;
};

namespace
{
rviz_rendering::RenderWindow *renderWindow( rviz_common::DisplayContext *context )
{
  auto *render_panel = context->getViewManager()->getRenderPanel();
  return render_panel == nullptr ? nullptr : render_panel->getRenderWindow();
}

void makeMainContextCurrent( rviz_common::DisplayContext *context )
{
  auto *render_window = renderWindow( context );
  if ( render_window == nullptr )
    return;
  auto *viewport = rviz_rendering::RenderWindowOgreAdapter::getOgreViewport( render_window );
  if ( viewport != nullptr )
    makeRenderTargetContextCurrent( viewport->getTarget() );
}
} // namespace

CameraOverlay::CameraOverlay( QQuickItem *parent )
    : QQuickItem( parent ), listener_( std::make_unique<Listener>( this ) )
{
  setFlag( ItemHasContents );
}

CameraOverlay::~CameraOverlay()
{
  if ( context_ == nullptr )
    return;
  if ( auto *root = Ogre::Root::getSingletonPtr() )
    root->removeFrameListener( listener_.get() );
  destroyTexture();
  auto *scene_manager = context_->getSceneManager();
  scene_manager->destroySceneNode( camera_node_ );
  scene_manager->destroyLight( light_ );
  scene_manager->destroyCamera( camera_ );
}

void CameraOverlay::componentComplete()
{
  QQuickItem::componentComplete();
  auto *qml_context = qmlContext( this );
  auto *rviz = qml_context == nullptr
                   ? nullptr
                   : qobject_cast<QmlRvizContext *>(
                         qml_context->contextProperty( "rviz" ).value<QObject *>() );
  if ( rviz == nullptr ) {
    LOG_ERROR( "CameraOverlay requires the rviz context property!" );
    return;
  }
  context_ = rviz->displayContext();
  auto *scene_manager = context_->getSceneManager();
  const std::string name =
      "hector_rviz_overlay/CameraOverlay/" + std::to_string( reinterpret_cast<uintptr_t>( this ) );
  camera_ = scene_manager->createCamera( name );
  camera_->setNearClipDistance( 0.05 );
  camera_->setFarClipDistance( 200 );
  camera_node_ = scene_manager->getRootSceneNode()->createChildSceneNode();
  camera_node_->attachObject( camera_ );
  light_ = scene_manager->createLight( name + "/Light" );
  light_->setType( Ogre::Light::LT_DIRECTIONAL );
  light_->setVisible( false );
  camera_node_->attachObject( light_ );
  Ogre::Root::getSingleton().addFrameListener( listener_.get() );
  requestRender();
}

void CameraOverlay::geometryChange( const QRectF &new_geometry, const QRectF &old_geometry )
{
  QQuickItem::geometryChange( new_geometry, old_geometry );
  requestRender();
}

void CameraOverlay::itemChange( ItemChange change, const ItemChangeData &value )
{
  QQuickItem::itemChange( change, value );
  if ( change == ItemVisibleHasChanged )
    requestRender();
}

void CameraOverlay::requestRender()
{
  if ( context_ != nullptr )
    context_->queueRender();
}

void CameraOverlay::prepareFrame()
{
  const bool active = isVisible() && window() != nullptr && !displays_.isEmpty() &&
                      !frame_id_.isEmpty() && intrinsics_.x() > 0 && intrinsics_.y() > 0 &&
                      !image_size_.isEmpty();
  if ( !active ) {
    if ( texture_ )
      texture_->getBuffer()->getRenderTarget()->setActive( false );
    if ( rendered_ ) {
      rendered_ = false;
      update();
    }
    return;
  }
  makeMainContextCurrent( context_ );
  updateTexture();
  if ( texture_ )
    texture_->getBuffer()->getRenderTarget()->setActive( true );
}

void CameraOverlay::preRender()
{
  Ogre::Vector3 position;
  Ogre::Quaternion orientation;
  auto *frame_manager = context_->getFrameManager();
  const std::string frame = frame_id_.toStdString();
  const rclcpp::Time stamp( static_cast<int64_t>( stamp_ * 1e9 ), RCL_ROS_TIME );
  const bool valid =
      ( stamp_ > 0 && frame_manager->getTransform( frame, stamp, position, orientation ) ) ||
      frame_manager->getTransform( frame, position, orientation );
  // Ogre cameras look along -z with y up, optical frames along z with y down.
  camera_node_->setPosition( position );
  camera_node_->setOrientation( orientation *
                                Ogre::Quaternion( Ogre::Degree( 180 ), Ogre::Vector3::UNIT_X ) );
  updateProjection();
  viewport_->setVisibilityMask( valid ? 0xFFFFFFFF : 0 );

  // rviz lights the scene along the main view, light it along this camera instead.
  main_light_ =
      rviz_rendering::RenderWindowOgreAdapter::getDirectionalLight( renderWindow( context_ ) );
  if ( main_light_ != nullptr ) {
    main_light_visible_ = main_light_->getVisible();
    main_light_->setVisible( false );
    light_->setDiffuseColour( main_light_->getDiffuseColour() );
    light_->setSpecularColour( main_light_->getSpecularColour() );
  }
  light_->setVisible( true );

  hidden_nodes_.clear();
  collectHiddenNodes( context_->getRootDisplayGroup(), QString() );
  auto *root = context_->getSceneManager()->getRootSceneNode();
  for ( auto *node : hidden_nodes_ ) root->removeChild( node );
}

void CameraOverlay::postRender()
{
  auto *root = context_->getSceneManager()->getRootSceneNode();
  for ( auto *node : hidden_nodes_ ) root->addChild( node );
  hidden_nodes_.clear();
  light_->setVisible( false );
  if ( main_light_ != nullptr )
    main_light_->setVisible( main_light_visible_ );
  texture_->getBuffer()->getRenderTarget()->copyContentsToMemory(
      Ogre::Box( 0, 0, image_.width(), image_.height() ),
      Ogre::PixelBox( image_.width(), image_.height(), 1, Ogre::PF_BYTE_RGBA, image_.bits() ) );
  rendered_ = true;
  update();
}

void CameraOverlay::collectHiddenNodes( rviz_common::DisplayGroup *group, const QString &prefix )
{
  auto *root = context_->getSceneManager()->getRootSceneNode();
  for ( int i = 0; i < group->numDisplays(); ++i ) {
    auto *display = group->getDisplayAt( i );
    const QString path = prefix + display->getName();
    if ( auto *child_group = qobject_cast<rviz_common::DisplayGroup *>( display ) ) {
      collectHiddenNodes( child_group, path + "/" );
      continue;
    }
    auto *node = display->getSceneNode();
    if ( node != nullptr && node->getParent() == root && !displays_.contains( path ) )
      hidden_nodes_.push_back( node );
  }
}

void CameraOverlay::updateProjection()
{
  constexpr float near = 0.05f;
  constexpr float far = 200.f;
  const float width = image_size_.width();
  const float height = image_size_.height();
  Ogre::Matrix4 projection = Ogre::Matrix4::ZERO;
  projection[0][0] = 2 * intrinsics_.x() / width;
  projection[1][1] = 2 * intrinsics_.y() / height;
  projection[0][2] = 1 - 2 * intrinsics_.z() / width;
  projection[1][2] = 2 * intrinsics_.w() / height - 1;
  projection[2][2] = -( far + near ) / ( far - near );
  projection[2][3] = -2 * far * near / ( far - near );
  projection[3][2] = -1;
  camera_->setCustomProjectionMatrix( true, projection );
}

void CameraOverlay::updateTexture()
{
  const QSizeF scene_size =
      mapRectToScene( boundingRect() ).size() * window()->effectiveDevicePixelRatio();
  const QSize size( std::max( 1, qRound( scene_size.width() ) ),
                    std::max( 1, qRound( scene_size.height() ) ) );
  if ( texture_ && size == image_.size() )
    return;
  destroyTexture();
  try {
    texture_ = Ogre::TextureManager::getSingleton().createManual(
        camera_->getName() + "/Texture", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D, size.width(), size.height(), 0, Ogre::PF_BYTE_RGBA,
        Ogre::TU_RENDERTARGET );
  } catch ( std::exception &ex ) {
    LOG_ERROR( "Failed to create camera overlay texture: %s", ex.what() );
    texture_.reset();
    return;
  }
  auto *target = texture_->getBuffer()->getRenderTarget();
  target->addListener( listener_.get() );
  viewport_ = target->addViewport( camera_ );
  viewport_->setClearEveryFrame( true );
  viewport_->setBackgroundColour( Ogre::ColourValue::ZERO );
  viewport_->setOverlaysEnabled( false );
  viewport_->setSkiesEnabled( false );
  viewport_->setShadowsEnabled( false );
  image_ = QImage( size, QImage::Format_RGBA8888_Premultiplied );
}

void CameraOverlay::destroyTexture()
{
  if ( !texture_ )
    return;
  texture_->getBuffer()->getRenderTarget()->removeAllListeners();
  Ogre::TextureManager::getSingleton().remove( texture_ );
  texture_.reset();
  viewport_ = nullptr;
  rendered_ = false;
}

QSGNode *CameraOverlay::updatePaintNode( QSGNode *node, UpdatePaintNodeData * )
{
  if ( !rendered_ ) {
    delete node;
    return nullptr;
  }
  auto *texture_node = static_cast<QSGSimpleTextureNode *>( node );
  if ( texture_node == nullptr ) {
    texture_node = new QSGSimpleTextureNode;
    texture_node->setOwnsTexture( true );
  }
  texture_node->setTexture(
      window()->createTextureFromImage( image_, QQuickWindow::TextureHasAlphaChannel ) );
  texture_node->setRect( boundingRect() );
  return texture_node;
}

QString CameraOverlay::frameId() const { return frame_id_; }

void CameraOverlay::setFrameId( const QString &value )
{
  if ( frame_id_ == value )
    return;
  frame_id_ = value;
  emit frameIdChanged();
  requestRender();
}

QVector4D CameraOverlay::intrinsics() const { return intrinsics_; }

void CameraOverlay::setIntrinsics( const QVector4D &value )
{
  if ( intrinsics_ == value )
    return;
  intrinsics_ = value;
  emit intrinsicsChanged();
  requestRender();
}

qreal CameraOverlay::stamp() const { return stamp_; }

void CameraOverlay::setStamp( qreal value )
{
  if ( stamp_ == value )
    return;
  stamp_ = value;
  emit stampChanged();
  requestRender();
}

QSize CameraOverlay::imageSize() const { return image_size_; }

void CameraOverlay::setImageSize( const QSize &value )
{
  if ( image_size_ == value )
    return;
  image_size_ = value;
  emit imageSizeChanged();
  requestRender();
}

QStringList CameraOverlay::displays() const { return displays_; }

void CameraOverlay::setDisplays( const QStringList &value )
{
  if ( displays_ == value )
    return;
  displays_ = value;
  emit displaysChanged();
  requestRender();
}
} // namespace hector_rviz_overlay
