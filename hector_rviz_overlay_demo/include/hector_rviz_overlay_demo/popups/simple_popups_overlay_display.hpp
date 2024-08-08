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

#ifndef HECTOR_RVIZ_OVERLAY_DEMO_SIMPLE_POPUPS_OVERLAY_DISPLAY_H
#define HECTOR_RVIZ_OVERLAY_DEMO_SIMPLE_POPUPS_OVERLAY_DISPLAY_H

#include <hector_rviz_overlay/displays/qwidget_overlay_display.hpp>

#include <hector_rviz_overlay/popup/qwidget_popup_factory.hpp>

#include "ui_demo_widget.h"

namespace hector_rviz_overlay_demo
{

class SimplePopupsOverlayDisplay : public hector_rviz_overlay::QWidgetOverlayDisplay
{
Q_OBJECT

protected slots:
  void onButtonClicked();

protected:
  void onSetupUi( QWidget *widget ) override;

  Ui::DemoWidget ui_;
  hector_rviz_overlay::QWidgetPopupOverlayPtr popup_;
};

}

#endif //HECTOR_RVIZ_OVERLAY_DEMO_SIMPLE_POPUPS_OVERLAY_DISPLAY_H
