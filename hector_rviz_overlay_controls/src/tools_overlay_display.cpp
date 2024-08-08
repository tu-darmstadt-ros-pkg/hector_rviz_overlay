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

#include "hector_rviz_overlay_controls/tools_overlay_display.h"

#include "hector_rviz_overlay_controls/tools_widget.h"

#include <QGridLayout>

#include <rviz_common/properties/string_property.hpp>
#include <rviz_common/properties/editable_enum_property.hpp>
#include <rviz_common/display_context.hpp>
#include <rviz_common/render_panel.hpp>
#include <rviz_common/tool_manager.hpp>
#include <rviz_common/view_manager.hpp>

using namespace hector_rviz_overlay;

namespace hector_rviz_overlay_controls
{

ToolsOverlayDisplay::ToolsOverlayDisplay()
  : QWidgetOverlayDisplay( false )
{
  show_names_property_ = new BoolProperty( "Show Names", true,
                                                 "Determines whether or not the names of the tools are shown in the buttons.",
                                                 this, SLOT( onShowNamesChanged()));
  style_sheet_property_->addOption( "package://hector_rviz_overlay_controls/styles/flat.qss" );
  style_sheet_property_->addOption( "package://hector_rviz_overlay_controls/styles/round.qss" );
  style_sheet_property_->setString( "package://hector_rviz_overlay_controls/styles/round.qss" );
}

void ToolsOverlayDisplay::onShowNamesChanged()
{
  tools_widget_->setNamesVisible( show_names_property_->getBool());
}

void ToolsOverlayDisplay::onSetupUi( QWidget *widget )
{
  grid_layout_ = new QGridLayout;

  grid_layout_->setRowStretch( 0, 1 );
  grid_layout_->setColumnStretch( 1, 1 );
  grid_layout_->addWidget( tools_widget_ = new ToolsWidget(), 1, 0 );
  tools_widget_->initializeTools( context_->getToolManager());
  grid_layout_->setAlignment( tools_widget_, Qt::AlignBottom );
  widget->setLayout( grid_layout_ );
}
}

#include <pluginlib/class_list_macros.hpp>
PLUGINLIB_EXPORT_CLASS( hector_rviz_overlay_controls::ToolsOverlayDisplay, rviz_common::Display )
