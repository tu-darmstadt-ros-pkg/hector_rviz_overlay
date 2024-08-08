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

#include "hector_rviz_overlay/popup/popup_container_widget.hpp"

#include "hector_rviz_overlay/popup/positioning/center_tracker.hpp"

#include <QEvent>
#include <QPainter>


namespace hector_rviz_overlay
{

PopupContainerWidget::PopupContainerWidget()
  : popup_( nullptr ), is_modal_( false ), modal_color_( QColor( 80, 80, 80, 180 ))
    , anchor_point_( AnchorCenter ), tracker_( std::make_shared<CenterTracker>()), last_position_()
{
}

QWidget *PopupContainerWidget::popup()
{
  return popup_;
}

const QWidget *PopupContainerWidget::popup() const
{
  return popup_;
}

void PopupContainerWidget::setPopup( QWidget *value )
{
  if ( popup_ != nullptr && popup_->parentWidget() == this )
  {
    popup_->setParent( nullptr );
  }
  popup_ = value;
  popup_->setParent( this );
  popup_->setGeometry( popup_->geometry().left(), popup_->geometry().top(),
                       popup_->sizeHint().width(), popup_->sizeHint().height());
}

bool PopupContainerWidget::event( QEvent *event )
{
  switch ( event->type())
  {
    case QEvent::LayoutRequest:
      popup_->setGeometry( popup_->geometry().left(), popup_->geometry().top(),
                           popup_->sizeHint().width(), popup_->sizeHint().height());
      break;
    default:
      break;
  }
  return QWidget::event( event );
}

void PopupContainerWidget::paintEvent( QPaintEvent *event )
{
  if ( popup_ == nullptr ) return;

  if ( tracker_ != nullptr )
  {
    QPoint pos = tracker_->getPoint( geometry().width(), geometry().height());
    if ( pos != last_position_ )
    {
      // TODO Account for AnchorAuto
      int x = pos.x();
      int y = pos.y();
      if ( anchor_point_ == AnchorCenter )
      {
        x = pos.x() - popup_->width() / 2;
        y = pos.y() - popup_->height() / 2;
      }
      else if ( anchor_point_ == AnchorTopRight )
      {
        x = pos.x() - popup_->width();
      }
      else if ( anchor_point_ == AnchorBottomRight )
      {
        x = pos.x() - popup_->width();
        y = pos.y() - popup_->height();
      }
      else if ( anchor_point_ == AnchorBottomLeft )
      {
        y = pos.y() - popup_->height();
      }


      QRect popup_geometry( x, y, popup_->width(), popup_->height());

      if ( popup_->geometry() != popup_geometry )
      {
        popup_->setGeometry( popup_geometry );
      }
      last_position_ = pos;
    }
  }

  QWidget::paintEvent( event );
  if ( !is_modal_ ) return;
  QPainter painter( this );
  painter.fillRect( 0, 0, geometry().width(), geometry().height(), modal_color_ );
}

bool PopupContainerWidget::isModalPopup() const
{
  return is_modal_;
}

void PopupContainerWidget::setIsModalPopup( bool value )
{
  is_modal_ = value;
}

const QColor &PopupContainerWidget::modalColor() const
{
  return modal_color_;
}

void PopupContainerWidget::setModalColor( const QColor &color )
{
  modal_color_ = color;
}

AnchorPoint PopupContainerWidget::anchorPoint() const
{
  return anchor_point_;
}

void PopupContainerWidget::setAnchorPoint( AnchorPoint value )
{
  anchor_point_ = value;
}

void PopupContainerWidget::trackPoint( std::shared_ptr<PointTracker> tracker )
{
  tracker_ = std::move( tracker );
}

QPoint PopupContainerWidget::popupPosition() const
{
  return popup_->pos();
}

void PopupContainerWidget::setPopupPosition( const QPoint &position )
{
  tracker_ = nullptr;
  int x = position.x();
  if ( x < 0 ) x = 0;
  else if ( x > geometry().width() - popup_->width()) x = geometry().width() - popup_->width();
  int y = position.y();
  if ( y < 0 ) y = 0;
  else if ( y > geometry().height() - popup_->height()) y = geometry().height() - popup_->height();
  popup_->setGeometry( x, y, popup_->geometry().width(), popup_->geometry().height());
}
}
