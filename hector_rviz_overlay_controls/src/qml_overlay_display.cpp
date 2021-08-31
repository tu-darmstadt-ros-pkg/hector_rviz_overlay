/*
 * Copyright (C) 2021  Stefan Fabian
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


#include "hector_rviz_overlay_controls/qml_overlay_display.h"
#include <rviz/properties/string_property.h>

namespace hector_rviz_overlay_controls
{
QmlOverlayDisplay::QmlOverlayDisplay() : hector_rviz_overlay::QmlOverlayDisplay( true )
{
  path_property_ = new rviz::StringProperty( "Path", "",
                                             "The path to the qml file that should be loaded. "
                                             "May use 'package://{PACKAGE_NAME}/' to refer to package relative paths",
                                             this, SLOT( onPathChanged()));
}

void QmlOverlayDisplay::onPathChanged()
{
  if ( qml_overlay_ == nullptr ) return;
  qml_overlay_->load( path_property_->getString());
}

QString QmlOverlayDisplay::getPathToQml()
{
  return path_property_->getString();
}
}

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS( hector_rviz_overlay_controls::QmlOverlayDisplay, rviz::Display )
