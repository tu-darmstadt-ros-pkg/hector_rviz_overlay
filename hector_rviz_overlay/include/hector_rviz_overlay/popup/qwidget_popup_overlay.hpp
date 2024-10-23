/*
 * Copyright (C) 2018  Stefan Fabian
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

#ifndef HECTOR_RVIZ_OVERLAY_QWIDGET_POPUP_OVERLAY_H
#define HECTOR_RVIZ_OVERLAY_QWIDGET_POPUP_OVERLAY_H

#include "hector_rviz_overlay/popup/popup_overlay.hpp"

#include "hector_rviz_overlay/popup/positioning/anchor_point.hpp"

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace hector_rviz_overlay
{
class PopupContainerWidget;
class QWidgetEventManager;

/*!
 * A QWidget based implementation of a PopupOverlay.
 */
class QWidgetPopupOverlay : public PopupOverlay
{
  Q_OBJECT
public:
  explicit QWidgetPopupOverlay( const std::string &name );

  ~QWidgetPopupOverlay() override;

  ///@inherit
  void prepareRender( Renderer * ) override { }

  ///@inherit
  void releaseRenderResources() override { }

  ///@inherit
  bool handleEvent( QObject *receiver, QEvent *event ) override;

  ///@inherit
  void handleEventsCanceled() override;

  ///@{
  /*!
   * @return The top level container widget that is drawn on top of the rviz_common::RenderPanel and
   * which manages the popup as its single child widget.
   */
  PopupContainerWidget *popupContainerWidget();

  const PopupContainerWidget *popupContainerWidget() const;
  ///@}

  ///@inherit
  void setScale( float value ) override;

  ///@inherit
  void setGeometry( const QRect &value ) override;

  /*!
   * A light dismissable popup can be closed by clicking anywhere outside the popup.
   *
   * @return Whether or not the popup in this overlay is light dismissable.
   */
  bool isLightDismissable() const;

  /*!
   * A light dismissable popup can be closed by clicking anywhere outside the popup.
   *
   * @param value Whether or not the popup in this popup overlay is light dismissable.
   */
  void setIsLightDismissable( bool value );

  bool isDirty() const override;

protected:
  ///@inherit
  void renderImpl( Renderer *renderer ) override;

  PopupContainerWidget *widget_;
  QWidgetEventManager *event_manager_;

  bool mouse_down_outside_popup_;

  bool light_dismissable_;
};

typedef std::shared_ptr<QWidgetPopupOverlay> QWidgetPopupOverlayPtr;
typedef std::shared_ptr<const QWidgetPopupOverlay> QWidgetPopupOverlayConstPtr;
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_QWIDGET_POPUP_OVERLAY_H
