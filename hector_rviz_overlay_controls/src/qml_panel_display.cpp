/*
 * Copyright (C) 2025  Stefan Fabian
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

#include "hector_rviz_overlay_controls/qml_panel_display.h"
#include <rviz_common/properties/string_property.hpp>

using namespace rviz_common::properties;

namespace hector_rviz_overlay_controls
{
QmlPanelDisplay::QmlPanelDisplay()
{
  path_property_ =
      new StringProperty( "Path", "",
                          "The path to the qml file that should be loaded. "
                          "May use 'package://{PACKAGE_NAME}/' to refer to package relative paths",
                          this, SLOT( onPathChanged() ) );
}

void QmlPanelDisplay::onPathChanged() { requestReload(); }

QString QmlPanelDisplay::getPathToQml()
{
  QString path = path_property_->getString();
  if ( !path.isEmpty() )
    return path;
  // Not written back into the path property, so an empty path is still saved as empty.
  return "package://hector_rviz_overlay_controls/media/qml_panel_placeholder.qml";
}
} // namespace hector_rviz_overlay_controls

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS( hector_rviz_overlay_controls::QmlPanelDisplay, rviz_common::Display )
