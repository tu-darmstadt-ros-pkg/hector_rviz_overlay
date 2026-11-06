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

#ifndef HECTOR_RVIZ_OVERLAY_CAMERA_OVERLAY_HPP
#define HECTOR_RVIZ_OVERLAY_CAMERA_OVERLAY_HPP

#include <OgreTexture.h>
#include <QImage>
#include <QQuickItem>
#include <QVector4D>

#include <memory>

namespace Ogre
{
class Camera;
class Light;
class SceneNode;
class Viewport;
} // namespace Ogre

namespace rviz_common
{
class DisplayContext;
class DisplayGroup;
} // namespace rviz_common

namespace hector_rviz_overlay
{

/*!
 * @brief Renders the rviz scene from the perspective of a pinhole camera with a transparent
 *   background, e.g., to draw markers on top of a camera image.
 *
 * Only the displays listed in displays are rendered.
 * Only works inside a QML overlay or panel that provides the rviz context property.
 */
class CameraOverlay : public QQuickItem
{
  Q_OBJECT
  //! The optical frame of the camera.
  Q_PROPERTY( QString frameId READ frameId WRITE setFrameId NOTIFY frameIdChanged )
  //! The intrinsics fx, fy, cx, cy in pixels of imageSize, e.g., the K matrix of the CameraInfo.
  Q_PROPERTY( QVector4D intrinsics READ intrinsics WRITE setIntrinsics NOTIFY intrinsicsChanged )
  //! The image size the intrinsics refer to.
  Q_PROPERTY( QSize imageSize READ imageSize WRITE setImageSize NOTIFY imageSizeChanged )
  //! The capture time of the shown image in seconds (ROS time) used to look up the camera pose.
  //! 0 or a failed lookup uses the latest pose.
  Q_PROPERTY( qreal stamp READ stamp WRITE setStamp NOTIFY stampChanged )
  //! The paths of the rendered displays, i.e., their names joined by '/' starting below the root
  //! group, e.g., Group/Markers.
  Q_PROPERTY( QStringList displays READ displays WRITE setDisplays NOTIFY displaysChanged )
public:
  explicit CameraOverlay( QQuickItem *parent = nullptr );

  ~CameraOverlay() override;

  QString frameId() const;

  void setFrameId( const QString &value );

  QVector4D intrinsics() const;

  void setIntrinsics( const QVector4D &value );

  QSize imageSize() const;

  void setImageSize( const QSize &value );

  qreal stamp() const;

  void setStamp( qreal value );

  QStringList displays() const;

  void setDisplays( const QStringList &value );

signals:

  void frameIdChanged();

  void intrinsicsChanged();

  void imageSizeChanged();

  void stampChanged();

  void displaysChanged();

protected:
  void componentComplete() override;

  void geometryChange( const QRectF &new_geometry, const QRectF &old_geometry ) override;

  void itemChange( ItemChange change, const ItemChangeData &value ) override;

  QSGNode *updatePaintNode( QSGNode *node, UpdatePaintNodeData * ) override;

private:
  class Listener;

  void requestRender();

  void prepareFrame();

  void preRender();

  void postRender();

  void updateProjection();

  void updateTexture();

  void destroyTexture();

  void collectHiddenNodes( rviz_common::DisplayGroup *group, const QString &prefix );

  rviz_common::DisplayContext *context_ = nullptr;
  std::unique_ptr<Listener> listener_;
  Ogre::SceneNode *camera_node_ = nullptr;
  Ogre::Camera *camera_ = nullptr;
  Ogre::TexturePtr texture_;
  Ogre::Viewport *viewport_ = nullptr;
  std::vector<Ogre::SceneNode *> hidden_nodes_;
  Ogre::Light *light_ = nullptr;
  Ogre::Light *main_light_ = nullptr;
  bool main_light_visible_ = true;

  QString frame_id_;
  QVector4D intrinsics_;
  QSize image_size_;
  qreal stamp_ = 0;
  QStringList displays_;
  QImage image_;
  bool rendered_ = false;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_CAMERA_OVERLAY_HPP
