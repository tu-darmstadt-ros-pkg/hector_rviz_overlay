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

#include "hector_rviz_overlay/events/qwidget_event_manager.hpp"

#include "hector_rviz_overlay/overlay_widget.hpp"

#include <QApplication>
#include <QMouseEvent>
#include <QWidget>

namespace hector_rviz_overlay
{

QWidgetEventManager::QWidgetEventManager( QWidget *widget )
  : widget_( widget )
{
}

QWidgetEventManager::~QWidgetEventManager() = default;

void QWidgetEventManager::handleEventsCanceled()
{
  QMouseEvent event( QEvent::MouseMove, hover_pos_, Qt::NoButton, Qt::NoButton, Qt::NoModifier );
  sendHoverEvents( nullptr, hover_pos_, &event );
  mouse_down_widget_ = nullptr;
}

bool QWidgetEventManager::handleEvent( QObject *receiver, QEvent *event )
{
  // Ignore events from our widgets to prevent stack overflow
  if ( receiver->isWidgetType())
  {
    if ( dynamic_cast<OverlayWidget *>(static_cast<QWidget *>(receiver)->topLevelWidget()))
    {
      return false;
    }
  }

  switch ( event->type())
  {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
      return handleMouseEvent( static_cast<QMouseEvent *>(event));
    case QEvent::Wheel:
      return handleWheelEvent( static_cast<QWheelEvent *>(event));
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      return handleKeyEvent( static_cast<QKeyEvent *>(event));
    default:
      break;
  }
  return false;
}

bool QWidgetEventManager::handleMouseEvent( QMouseEvent *event )
{
  // Map the mouse event to the render panel because the top left of the render panel is equivalent to
  // the top left of the widget. Using the widget to map does not work because Qt doesn't know where the widget is drawn.
  QPoint widget_pos = event->pos() / scale_;
  bool is_mouse_down = (event->buttons() & Qt::LeftButton) == Qt::LeftButton;
  if ( mouse_down_widget_ )
  {
    QPoint local_pos = mouse_down_widget_->mapFrom( widget_, widget_pos);

    QMouseEvent child_mouse_event( event->type(), local_pos, event->screenPos(),
                                   event->button(), event->buttons(), event->modifiers());
    QApplication::sendEvent( mouse_down_widget_, &child_mouse_event );

    if ( !is_mouse_down )
    {
      mouse_down_widget_ = nullptr;
    }

    event->setAccepted( true );
    return true;
  }

  QWidget *child = widget_->childAt( widget_pos );
  if ( child == nullptr )
  {
    sendHoverEvents( nullptr, widget_pos, event );
    return false;
  }


  // Now forward the event to the child with the position of the mouse relative to the child widget
  QMouseEvent child_mouse_event( event->type(), child->mapFrom( widget_, widget_pos ), event->screenPos(),
                                 event->button(), event->buttons(), event->modifiers());

  bool handled = QApplication::sendEvent( child, &child_mouse_event );
  if ( !handled || !child_mouse_event.isAccepted())
  {
    sendHoverEvents( nullptr, widget_pos, event );
    return false;
  }

  sendHoverEvents( child, widget_pos, event );

  // Set mouse down widget if mouse is down
  if ( event->type() == QEvent::MouseButtonPress && event->button() == Qt::LeftButton )
  {
    focus_widget_ = child;
    mouse_down_widget_ = child;
  }
  // Event is accepted because if the child event wasn't accepted, we wouldn't end up here
  event->setAccepted( true );
  return true;
}

bool QWidgetEventManager::handleWheelEvent( QWheelEvent *event )
{
  // Map the wheel event to the render panel because the top left of the render panel is equivalent to
  // the top left of the widget. Using the widget to map does not work because Qt doesn't know where the widget is drawn.
  QPointF local_pos = event->position() / scale_;
  QWidget *child = widget_->childAt( local_pos.toPoint() );
  if ( child == nullptr )
  {
    return false;
  }

  // Now forward the event to the child with the position of the mouse relative to the child widget
  local_pos = child->mapFrom( widget_, local_pos.toPoint() );
  QWheelEvent child_event( local_pos, event->globalPosition(), event->pixelDelta(), event->angleDelta(),
                           event->buttons(), event->modifiers(), event->phase(),
                           event->source());
  bool handled = QApplication::sendEvent( child, &child_event );
  if ( !handled || !child_event.isAccepted())
  {
    // This stops wheel events from passing to the rviz_common::RenderPanel if the widget is not scrollable
    // TODO: Maybe there's a better way to find out whether to pass on scroll events or not
    // Checking pixel color would be an option but very inefficient
    QWidget *widget = mouse_over_widget_;
    while (widget != nullptr)
    {
      if (widget->property("IgnoreWheelEvents").isNull() || !widget->property("IgnoreWheelEvents").toBool())
      {
        return dynamic_cast<OverlayWidget *>(widget) == nullptr;
      }
      widget = widget->parentWidget();
    }
    return false;
  }
  event->setAccepted( true );
  return true;
}

bool QWidgetEventManager::handleKeyEvent( QKeyEvent *event )
{
  if ( focus_widget_ == nullptr ) return false;
// #ifdef RENDER_OVERLAYS_USING_OPENGL
//   return false;
// #else
  return QApplication::sendEvent( focus_widget_, event ) && event->isAccepted();
// #endif
}

void QWidgetEventManager::sendHoverEvents( QWidget *child, QPoint pos, QMouseEvent *event )
{
  // If mouse over widget changed, send enter/leave events
  if ( mouse_over_widget_ != child )
  {
    QWidgetList enter_list;
    QWidgetList leave_list;

    // Find the first parent that both the last mouse over widget and the child share
    // First determine the depth of the last and the new
    int enter_depth = 0;
    int leave_depth = 0;
    for ( QWidget *w = mouse_over_widget_; w != nullptr; w = w->parentWidget()) ++leave_depth;
    for ( QWidget *w = child; w != nullptr; w = w->parentWidget()) ++enter_depth;

    // Now go up both hierarchies until we have parents of the same depth
    QWidget *leave_parent = mouse_over_widget_;
    for ( ; leave_depth > enter_depth; --leave_depth )
    {
      leave_parent = leave_parent->parentWidget();
    }
    QWidget *enter_parent = child;
    for ( ; enter_depth > leave_depth; --enter_depth )
    {
      enter_parent = enter_parent->parentWidget();
    }

    // Finally, move up in conjunction until we find the shared parent
    while ( enter_parent != leave_parent )
    {
      enter_parent = enter_parent->parentWidget();
      leave_parent = leave_parent->parentWidget();
    }

    // Now, collect all parents of the old and the new until the shared parent
    for ( QWidget *w = mouse_over_widget_; w != leave_parent; w = w->parentWidget())
    {
      leave_list.append( w );
    }
    for ( QWidget *w = child; w != enter_parent; w = w->parentWidget())
    {
      enter_list.append( w );
    }

    // Send the leave event to all widgets that were determined
    QEvent leave_event( QEvent::Leave );
    for ( int k = 0; k < leave_list.size(); ++k )
    {
      QWidget *w = leave_list.at( k );
      // TODO: Maybe modal widget checks
      QApplication::sendEvent( w, &leave_event );
      // If the widget has the hover attribute, send it a HoverLeave event as well
      if ( w->testAttribute( Qt::WA_Hover ))
      {
        QHoverEvent hover_leave_event( QEvent::HoverLeave, QPoint( -1, -1 ),
                                       w->mapFrom( w->topLevelWidget(), hover_pos_ ), event->modifiers());
        QApplication::sendEvent( w, &hover_leave_event );
      }
    }

    // Send the enter event to all widgets that were determined
    for ( int k = 0; k < enter_list.size(); ++k )
    {
      QWidget *w = enter_list.at( k );
      QPoint local_pos = w->mapFrom( widget_, pos );
      QEnterEvent enter_event( local_pos, event->windowPos(), event->screenPos());
      QApplication::sendEvent( w, &enter_event );
      // If the widget has the hover attribute, send it a HoverEnter event as well
      if ( w->testAttribute( Qt::WA_Hover ))
      {
        QHoverEvent hover_enter_event( QEvent::HoverEnter, local_pos, QPoint( -1, -1 ), event->modifiers());
        QApplication::sendEvent( w, &hover_enter_event );
      }
    }
    mouse_over_widget_ = child;
  }
  hover_pos_ = pos;
}

float QWidgetEventManager::scale() const
{
  return scale_;
}

void QWidgetEventManager::setScale( float value )
{
  scale_ = value;
}
}
