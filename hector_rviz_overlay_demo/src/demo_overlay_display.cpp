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

#include "hector_rviz_overlay_demo/demo_overlay_display.hpp"

using namespace hector_rviz_overlay;

namespace hector_rviz_overlay_demo
{

void DemoOverlayDisplay::onSetupUi( QWidget *widget )
{
  ui_.setupUi( widget );
  connect( ui_.pushButton, SIGNAL( clicked()), this, SLOT( onButtonClicked()));
}

void DemoOverlayDisplay::onButtonClicked()
{
  if ( ui_.pushButton->text() == "Push Me" )
  {
    ui_.pushButton->setText( "Push Me Again" );
  }
  else if ( ui_.pushButton->text() == "Push Me Again" )
  {
    ui_.pushButton->setText( "Once More" );
  }
  else if ( ui_.pushButton->text() == "Once More" )
  {
    ui_.pushButton->setText( "Okay. Enough." );
  }
  else if ( ui_.pushButton->text() == "Okay. Enough." )
  {
    ui_.pushButton->setText( "Stop it!" );
  }
  else if ( ui_.pushButton->text() == "Stop it!" )
  {
    ui_.pushButton->setText( "Last warning!" );
  }
  else if ( ui_.pushButton->text() == "Last warning!" )
  {
    ui_.pushButton->setText( "3" );
  }
  else if ( ui_.pushButton->text() == "3" )
  {
    ui_.pushButton->setText( "2" );
  }
  else if ( ui_.pushButton->text() == "2" )
  {
    ui_.pushButton->setText( "1" );
  }
  else if ( ui_.pushButton->text() == "1" )
  {
    if (system( "xdg-open https://www.youtube.com/watch?v=dQw4w9WgXcQ" ) != 0)
    {
      ui_.pushButton->setText( "Failed to Rick-Roll :(" );
      return;
    }
    ui_.pushButton->setText( "Push Me" );
  }
}
}

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS( hector_rviz_overlay_demo::DemoOverlayDisplay, rviz_common::Display )
