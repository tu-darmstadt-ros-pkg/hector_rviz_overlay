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

#ifndef HECTOR_RVIZ_OVERLAY_QWIDGET_EVENT_MANAGER_H
#define HECTOR_RVIZ_OVERLAY_QWIDGET_EVENT_MANAGER_H

#include <QPoint>

class QEvent;
class QKeyEvent;
class QMouseEvent;
class QObject;
class QWheelEvent;
class QWidget;

namespace hector_rviz_overlay
{

/*!
 * Manages the event passing for QWidgets.
 * This includes sending hover events.
 */
class QWidgetEventManager
{
public:
  explicit QWidgetEventManager(QWidget *widget);

  virtual ~QWidgetEventManager();

  float scale() const;

  void setScale(float value);

  bool handleEvent( QObject *receiver, QEvent *event );

  void handleEventsCanceled();

protected:

  bool handleMouseEvent( QMouseEvent *event );

  bool handleWheelEvent( QWheelEvent *event );

  bool handleKeyEvent( QKeyEvent *event );

  void sendHoverEvents( QWidget *child, QPoint pos, QMouseEvent *event );

  QWidget *widget_;
  float scale_;

  QWidget *mouse_over_widget_;
  QPoint hover_pos_;
  QWidget *mouse_down_widget_;
  QWidget *focus_widget_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_QWIDGET_EVENT_MANAGER_H
