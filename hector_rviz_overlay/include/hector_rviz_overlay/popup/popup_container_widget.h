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

#ifndef HECTOR_RVIZ_OVERLAY_POPUP_CONTAINER_WIDGET_H
#define HECTOR_RVIZ_OVERLAY_POPUP_CONTAINER_WIDGET_H

#include "hector_rviz_overlay/overlay_widget.h"

#include "hector_rviz_overlay/popup/positioning/anchor_point.h"
#include "hector_rviz_overlay/popup/positioning/point_tracker.h"

#include <QPoint>

#include <memory>

namespace hector_rviz_overlay
{

/*!
 * A container widget for a single popup which manages the position of the popup and filling the background if the popup
 * is modal.
 */
class PopupContainerWidget : public OverlayWidget
{
Q_OBJECT
public:
  PopupContainerWidget();

  /*!
   * The popup widget is usually a dialog which should not contain the entire screen.
   * This container manages filling the background if it is modal and the position on the screen e.g. if it should be
   * fixed to a 3D location.
   * @return The popup that is managed by this container widget.
   */
  QWidget *popup();

  /*!
   * @see popup()
   * @return The popup that is managed by this container widget.
   */
  const QWidget *popup() const;

  /*!
   * @see popup()
   * @param value The popup that should be managed by this container widget.
   */
  void setPopup( QWidget *value );

  /*!
   * @return The position of the popup widget's top left corner.
   */
  QPoint popupPosition() const;

  /*!
   * Sets the position of the popup widget's top left corner.
   * This automatically removes the PointTracker if there is one.
   * Note: This is fixed to the top left corner. The anchorPoint property is ignored!
   *
   * @param position The position of the popup widget's top left corner.
   */
  void setPopupPosition( const QPoint &position );

  /*!
   * A modal popup fills the background with a color specified by modalColor() and catches all MouseEvents (even outside
   * the popup's bounds)
   * @return Whether or not the popup is modal.
   */
  bool isModalPopup() const;

  /*!
   * @see isModalPopup()
   * @param value Whether or not the popup should be modal.
   */
  void setIsModalPopup( bool value );

  /*!
   * @see isModalPopup()
   * @return The color that is used to fill the screen outside of the popup.
   */
  const QColor &modalColor() const;

  /*!
   * @see isModalPopup()
   * @param color The color that is used to fill the screen outside of the popup.
   */
  void setModalColor( const QColor &color );

  /*!
   * E.g. if the anchor is set to AnchorBottomRight, the bottom right of the popup is set to the position provided by
   * the PointTracker.
   * The popupPosition property ignores this property as it is always fixed to the top left.
   *
   * @return The anchor to which the position of the tracker is set.
   */
  AnchorPoint anchorPoint() const;

  /*!
   * E.g. if the anchor is set to AnchorBottomRight, the bottom right of the popup is set to the position provided by
   * the PointTracker.
   * The popupPosition property ignores this property as it is always fixed to the top left.
   *
   * @param value The anchor to which the position of the tracker is set.
   */
  void setAnchorPoint( AnchorPoint value );

  /*!
   * Sets the popup to position itself according to the point given by the passed PointTracker.
   * The position of the point relative to the bottom is given by the anchorPoint property.
   *
   * @param tracker The point tracker that returns the position for the popup on demand.
   */
  void trackPoint( std::shared_ptr<PointTracker> tracker );

protected:

  void paintEvent( QPaintEvent *event ) override;

  bool event( QEvent *event ) override;

  QWidget *popup_;

  bool is_modal_;
  QColor modal_color_;
  AnchorPoint anchor_point_;
  std::shared_ptr<PointTracker> tracker_;
  QPoint last_position_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_POPUP_CONTAINER_WIDGET_H
