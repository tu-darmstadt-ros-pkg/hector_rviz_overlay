/*
 * Copyright (C) 2019  Stefan Fabian
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

#ifndef HECTOR_RVIZ_OVERLAY_DEMO_DEMO_QML_ROS_OVERLAY_DISPLAY_H
#define HECTOR_RVIZ_OVERLAY_DEMO_DEMO_QML_ROS_OVERLAY_DISPLAY_H

#include <hector_rviz_overlay/displays/qml_overlay_display.h>

namespace hector_rviz_overlay_demo
{

class DemoQmlRosOverlayDisplay : public hector_rviz_overlay::QmlOverlayDisplay
{
Q_OBJECT
public:
  DemoQmlRosOverlayDisplay() : hector_rviz_overlay::QmlOverlayDisplay() {}

protected:
  QString getPathToQml() override;
};

} // hector_rviz_overlay_demo

#endif //HECTOR_RVIZ_OVERLAY_DEMO_DEMO_QML_ROS_OVERLAY_DISPLAY_H
