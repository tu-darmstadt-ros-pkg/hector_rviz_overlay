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

#include "hector_rviz_overlay/popup/qwidget_popup_overlay.hpp"

#include "hector_rviz_overlay/events/qwidget_event_manager.hpp"
#include "hector_rviz_overlay/popup/popup_container_widget.hpp"
#include "hector_rviz_overlay/render/renderer.hpp"

#include <QEvent>
#include <QPainter>

namespace hector_rviz_overlay
{

QWidgetPopupOverlay::QWidgetPopupOverlay( const std::string &name )
  : PopupOverlay( name ), mouse_down_outside_popup_( false ), light_dismissable_( false )
{
  widget_ = new PopupContainerWidget;
  event_manager_ = new QWidgetEventManager( widget_ );
}

QWidgetPopupOverlay::~QWidgetPopupOverlay()
{
  delete event_manager_;
  delete widget_;
}

PopupContainerWidget *QWidgetPopupOverlay::popupContainerWidget() { return widget_; }

const PopupContainerWidget *QWidgetPopupOverlay::popupContainerWidget() const { return widget_; }

void QWidgetPopupOverlay::setScale( float value )
{
  Overlay::setScale( value );
  widget_->setGeometry( QRect( geometry().left(), geometry().top(),
                               (int) (geometry().width() / scale()), (int) (geometry().height() / scale())));
  event_manager_->setScale( value );
}

void QWidgetPopupOverlay::setGeometry( const QRect &value )
{
  Overlay::setGeometry( value );
  widget_->setGeometry( QRect( geometry().left(), geometry().top(),
                               (int) (geometry().width() / scale()), (int) (geometry().height() / scale())));
}

void QWidgetPopupOverlay::renderImpl( Renderer *renderer )
{
  QPainter painter( renderer->paintDevice());
  painter.scale( scale(), scale() );
  widget_->render( &painter );
}

void QWidgetPopupOverlay::handleEventsCanceled()
{
  event_manager_->handleEventsCanceled();
}

bool QWidgetPopupOverlay::handleEvent( QObject *receiver, QEvent *event )
{
  // Ignore events from our widgets as is done by the event manager
  if ( receiver->isWidgetType())
  {
    if ( dynamic_cast<OverlayWidget *>(static_cast<QWidget *>(receiver)->topLevelWidget()))
    {
      return false;
    }
  }

  event->setAccepted( false );
  bool handled = event_manager_->handleEvent( receiver, event );
  if ( handled || event->isAccepted())
  {
    if ( mouse_down_outside_popup_ )
    {
      mouse_down_outside_popup_ = event->type() != QEvent::MouseButtonRelease;
    }
    return true;
  }

  // If the popup is light dismissable, it should get closed when we click outside of it.
  // If it is modal, we wait for the press and release to occur outside the popup. If one of them is not outside, we don't close it.
  // If it is not modal, we close it immediately because we may not get the release event (the overlay consuming the
  //   press event will get the release event)
  if ( !light_dismissable_ ) return widget_->isModalPopup();

  if ( event->type() == QEvent::MouseButtonPress )
  {
    if ( !widget_->isModalPopup())
    {
      hide();
    }
    else
    {
      mouse_down_outside_popup_ = true;
    }
  }
  else if ( mouse_down_outside_popup_ && event->type() == QEvent::MouseButtonRelease )
  {
    mouse_down_outside_popup_ = false;
    hide();
  }
  return widget_->isModalPopup();
}

bool QWidgetPopupOverlay::isLightDismissable() const
{
  return light_dismissable_;
}

void QWidgetPopupOverlay::setIsLightDismissable( bool value )
{
  light_dismissable_ = value;
}

bool QWidgetPopupOverlay::isDirty() const
{
  // Widgets are always dirty since there is currently no reliable way of finding out whether that's true
  return true;
}
}
