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

#include "hector_rviz_overlay_demo/popups/simple_popups_overlay_display.h"

#include <hector_rviz_overlay/overlay_manager.h>

using namespace hector_rviz_overlay;

namespace hector_rviz_overlay_demo
{

void SimplePopupsOverlayDisplay::onSetupUi( QWidget *widget )
{
  ui_.setupUi( widget );
  connect( ui_.pushButton, SIGNAL( clicked()), this, SLOT( onButtonClicked()));

  popup_ = QWidgetPopupFactory().createMessagePopup("SimplePopup", "Test", "This is a test popup!\nThe message should be long and have a forced and maybe an automatic linebreak.");
  popup_->setIsLightDismissable( true );
  OverlayManager::getSingleton().addOverlay( popup_, true );
}

void SimplePopupsOverlayDisplay::onButtonClicked()
{
  popup_->show();
}
}

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS( hector_rviz_overlay_demo::SimplePopupsOverlayDisplay, rviz::Display )
