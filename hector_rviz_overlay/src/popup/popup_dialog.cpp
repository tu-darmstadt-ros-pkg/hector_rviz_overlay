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

#include "hector_rviz_overlay/popup/popup_dialog.hpp"

#include "hector_rviz_overlay/popup/positioning/popup_move_bar.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPushButton>

namespace hector_rviz_overlay
{

PopupDialog::PopupDialog( QWidget *parent, Qt::WindowFlags flags )
    : QFrame( parent, flags ), content_( nullptr )
{
  layout_ = new QGridLayout;
  label_ = new QLabel;
  label_->setObjectName( "Label" );
  label_->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );
  layout_->addWidget( label_, 0, 0 );

  auto move_bar = new PopupMoveBar;
  layout_->addWidget( move_bar, 0, 0 );

  // Buttons
  auto button_layout = new QHBoxLayout;
  button_yes_ = new QPushButton( "Yes" );
  connect( button_yes_, &QPushButton::clicked, this, &PopupDialog::yesClicked );
  button_layout->addWidget( button_yes_ );

  button_no_ = new QPushButton( "No" );
  connect( button_no_, &QPushButton::clicked, this, &PopupDialog::noClicked );
  button_layout->addWidget( button_no_ );

  button_ok_ = new QPushButton( "Ok" );
  connect( button_ok_, &QPushButton::clicked, this, &PopupDialog::okClicked );
  button_layout->addWidget( button_ok_ );

  button_cancel_ = new QPushButton( "Cancel" );
  connect( button_cancel_, &QPushButton::clicked, this, &PopupDialog::cancelClicked );
  button_layout->addWidget( button_cancel_ );
  button_layout->setMargin( 4 );

  layout_->addLayout( button_layout, 2, 0, Qt::AlignRight );

  layout_->setRowMinimumHeight( 0, 24 );
  layout_->setRowStretch( 1, 1 );
  layout_->setSpacing( 0 );
  layout_->setMargin( 0 );
  setLayout( layout_ );
  setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Minimum );

  setAttribute( Qt::WA_NoMousePropagation );
  setAttribute( Qt::WA_Hover );
  setStyleSheet( "hector_rviz_overlay--PopupDialog {"
                 " background-color: #dddddd;"
                 " border: 2px solid #aaaaaa;"
                 " border-radius: 6px;"
                 " min-height: 120px;"
                 " min-width: 320px;"
                 "}"
                 ""
                 "QLabel#Label { background-color: #aaaaaa; padding: 4px 8px; }"
                 ""
                 "#Content {"
                 " padding: 12px;"
                 "}" );
}

QWidget *PopupDialog::content() { return content_; }

const QWidget *PopupDialog::content() const { return content_; }

void PopupDialog::setContent( QWidget *widget )
{
  if ( content_ != nullptr ) {
    layout_->removeWidget( content_ );
  }
  content_ = widget;
  content_->setObjectName( "Content" );
  layout_->addWidget( content_, 1, 0 );
}

QString PopupDialog::title() const { return label_->text(); }

void PopupDialog::setTitle( const QString &value ) { label_->setText( value ); }

bool PopupDialog::event( QEvent *event )
{
  bool handled = QFrame::event( event );
  if ( handled && event->isAccepted() )
    return true;

  switch ( event->type() ) {
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::HoverEnter:
  case QEvent::HoverLeave:
  case QEvent::HoverMove:
  case QEvent::MouseMove:
    event->accept();
    return true;
  default:
    return handled;
  }
}

int PopupDialog::buttons() const { return buttons_; }

void PopupDialog::setButtons( int buttons )
{
  buttons_ = buttons;
  button_yes_->setVisible( ( buttons & PopupButtons::ButtonYes ) == PopupButtons::ButtonYes );
  button_no_->setVisible( ( buttons & PopupButtons::ButtonNo ) == PopupButtons::ButtonNo );
  button_ok_->setVisible( ( buttons & PopupButtons::ButtonOk ) == PopupButtons::ButtonOk );
  button_cancel_->setVisible( ( buttons & PopupButtons::ButtonCancel ) == PopupButtons::ButtonCancel );
}

void PopupDialog::yesClicked() { emit buttonClicked( ButtonYes ); }

void PopupDialog::noClicked() { emit buttonClicked( ButtonNo ); }

void PopupDialog::okClicked() { emit buttonClicked( ButtonOk ); }

void PopupDialog::cancelClicked() { emit buttonClicked( ButtonCancel ); }
} // namespace hector_rviz_overlay
