/*
 * Copyright (C) 2017  Stefan Fabian
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

#include "hector_rviz_overlay/ui/qwidget_overlay.h"

#include "hector_rviz_overlay/events/qwidget_event_manager.h"
#include "hector_rviz_overlay/render/renderer.h"
#include "hector_rviz_overlay/overlay_widget.h"

#include <QPainter>

namespace hector_rviz_overlay
{

QWidgetOverlay::QWidgetOverlay( const std::string &name )
  : UiOverlay( name )
{
  widget_ = new OverlayWidget;
  event_manager_ = new QWidgetEventManager( widget_ );
}

QWidgetOverlay::~QWidgetOverlay()
{
  delete event_manager_;
  delete widget_;
}

QWidget *QWidgetOverlay::widget() { return widget_; }

const QWidget *QWidgetOverlay::widget() const { return widget_; }

void QWidgetOverlay::setScale( float value )
{
  Overlay::setScale( value );
  widget_->setGeometry( QRect( geometry().left(), geometry().top(),
                               (int) (geometry().width() / scale()), (int) (geometry().height() / scale())));
  event_manager_->setScale( value );
}

void QWidgetOverlay::setGeometry( const QRect &value )
{
  Overlay::setGeometry( value );
  widget_->setGeometry( QRect( geometry().left(), geometry().top(),
                               (int) (geometry().width() / scale()), (int) (geometry().height() / scale())));
}

void QWidgetOverlay::renderImpl( Renderer *renderer )
{
  QPainter painter( renderer->paintDevice());
  painter.scale( scale(), scale() );
  widget_->render( &painter );
}

void QWidgetOverlay::handleEventsCanceled()
{
  event_manager_->handleEventsCanceled();
}

bool QWidgetOverlay::handleEvent( QObject *receiver, QEvent *event )
{
  return event_manager_->handleEvent( receiver, event );
}

bool QWidgetOverlay::isDirty() const
{
  // Widgets are always dirty since there is currently no reliable way of finding out whether that's true
  return true;
}
}
