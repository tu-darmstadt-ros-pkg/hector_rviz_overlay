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

#include <hector_rviz_overlay/displays/qwidget_overlay_display.hpp>

#include "ui_demo_widget.h"

namespace hector_rviz_overlay_demo
{

class DemoOverlayDisplay : public hector_rviz_overlay::QWidgetOverlayDisplay
{
  Q_OBJECT

protected slots:
  void onButtonClicked();

protected:
  void onSetupUi( QWidget *widget ) override;

  Ui::DemoWidget ui_;
};

}

#endif //HECTOR_RVIZ_OVERLAY_DEMO_DEMO_OVERLAY_DISPLAY_H
