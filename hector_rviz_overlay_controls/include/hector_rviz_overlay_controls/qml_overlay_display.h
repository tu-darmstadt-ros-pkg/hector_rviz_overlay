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

#ifndef HECTOR_RVIZ_OVERLAY_CONTROLS_QML_OVERLAY_DISPLAY_H
#define HECTOR_RVIZ_OVERLAY_CONTROLS_QML_OVERLAY_DISPLAY_H

#include <hector_rviz_overlay/displays/qml_overlay_display.hpp>

namespace rviz_common::properties
{
class StringProperty;
}

namespace hector_rviz_overlay_controls
{

class QmlOverlayDisplay : public hector_rviz_overlay::QmlOverlayDisplay
{
  Q_OBJECT
public:
  QmlOverlayDisplay();

private slots:
  void onPathChanged();

protected:
  QString getPathToQml() override;

private:
  rviz_common::properties::StringProperty *path_property_;
};
} // namespace hector_rviz_overlay_controls

#endif // HECTOR_RVIZ_OVERLAY_CONTROLS_QML_OVERLAY_DISPLAY_H
