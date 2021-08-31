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

#ifndef HECTOR_RVIZ_OVERLAY_DEMO_DEMO_OVERLAY_DISPLAY_H
#define HECTOR_RVIZ_OVERLAY_DEMO_DEMO_OVERLAY_DISPLAY_H

#include <hector_rviz_overlay/displays/qwidget_overlay_display.h>

class QGridLayout;

namespace hector_rviz_overlay_controls
{

class ToolsWidget;

class ToolsOverlayDisplay : public hector_rviz_overlay::QWidgetOverlayDisplay
{
Q_OBJECT
public:
  ToolsOverlayDisplay();

protected slots:
  void onShowNamesChanged();

  void onShowAddRemoveChanged();

protected:

  void onSetupUi( QWidget *widget ) override;

  rviz::BoolProperty *show_names_property_;
  rviz::BoolProperty *show_add_remove_buttons_property_;

  ToolsWidget *tools_widget_;
  QGridLayout *grid_layout_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_DEMO_DEMO_OVERLAY_DISPLAY_H
