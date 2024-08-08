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

#include "hector_rviz_overlay/popup/positioning/popup_move_bar.hpp"

#include "hector_rviz_overlay/popup/popup_container_widget.hpp"

#include <QMouseEvent>

namespace hector_rviz_overlay
{

PopupMoveBar::PopupMoveBar( QWidget *parent ) : QWidget( parent )
{
  setAttribute( Qt::WA_NoSystemBackground );
}

void PopupMoveBar::mousePressEvent( QMouseEvent *event )
{
  QWidget::mousePressEvent( event );

  auto container = dynamic_cast<PopupContainerWidget *>(topLevelWidget());
  if (container == nullptr) return;

  start_mouse_position_ = event->globalPos();
  start_position_ = container->popupPosition();
  event->accept();
}

void PopupMoveBar::mouseReleaseEvent( QMouseEvent *event )
{
  QWidget::mouseReleaseEvent( event );
  event->accept();
}

void PopupMoveBar::mouseMoveEvent( QMouseEvent *event )
{
  QWidget::mouseMoveEvent( event );

  auto container = dynamic_cast<PopupContainerWidget *>(topLevelWidget());
  if (container == nullptr) return;

  container->setPopupPosition(start_position_ + event->globalPos() - start_mouse_position_);
  event->accept();
}
}
