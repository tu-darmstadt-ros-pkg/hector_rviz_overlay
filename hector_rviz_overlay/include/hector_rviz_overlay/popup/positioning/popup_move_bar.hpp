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

#ifndef HECTOR_RVIZ_OVERLAY_POPUP_MOVE_BAR_H
#define HECTOR_RVIZ_OVERLAY_POPUP_MOVE_BAR_H

#include <QPoint>
#include <QWidget>

namespace hector_rviz_overlay
{

/*!
 * A widget that can be placed inside a popup to make it movable. Clicking and holding the mouse on the PopupMoveBar
 * will cause movements of the mouse to be reflected by the position of the popup.
 * This only has an effect if the popup is child of a PopupContainerWidget.
 * Moving a popup with a PopupMoveBar will cancel any PointTrackers that are used by the PopupContainerWidget.
 */
class PopupMoveBar : public QWidget
{
public:
  explicit PopupMoveBar(QWidget *parent = nullptr);

protected:
  void mousePressEvent( QMouseEvent *event ) override;

  void mouseReleaseEvent( QMouseEvent *event ) override;

  void mouseMoveEvent( QMouseEvent *event ) override;

protected:
  QPoint start_position_;
  QPoint start_mouse_position_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_POPUP_MOVE_BAR_H
