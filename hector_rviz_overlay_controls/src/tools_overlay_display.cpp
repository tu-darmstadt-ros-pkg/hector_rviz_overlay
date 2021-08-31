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

#include <rviz/properties/string_property.h>
#include <rviz/properties/editable_enum_property.h>
#include <rviz/display_context.h>
#include <rviz/render_panel.h>
#include <rviz/tool_manager.h>
#include <rviz/view_manager.h>
#include <rviz/frame_manager.h>

using namespace hector_rviz_overlay;

namespace hector_rviz_overlay_controls
{

ToolsOverlayDisplay::ToolsOverlayDisplay()
  : QWidgetOverlayDisplay( false ), grid_layout_( nullptr ), tools_widget_( nullptr )
{
  show_names_property_ = new rviz::BoolProperty( "Show Names", true,
                                                 "Determines whether or not the names of the tools are shown in the buttons.",
                                                 this, SLOT( onShowNamesChanged()));
  show_add_remove_buttons_property_ = new rviz::BoolProperty( "Show Add/Remove", true,
                                                              "Determines whether or not the add and remove buttons are shown.",
                                                              this, SLOT( onShowAddRemoveChanged()));
  style_sheet_property_->addOption( "package://hector_rviz_overlay_controls/styles/flat.qss" );
  style_sheet_property_->addOption( "package://hector_rviz_overlay_controls/styles/round.qss" );
  style_sheet_property_->setString( "package://hector_rviz_overlay_controls/styles/round.qss" );
}

void ToolsOverlayDisplay::onShowNamesChanged()
{
  tools_widget_->setNamesVisible( show_names_property_->getBool());
}

void ToolsOverlayDisplay::onShowAddRemoveChanged()
{
  tools_widget_->setAddRemoveButtonsVisible( show_add_remove_buttons_property_->getBool());
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

#include <pluginlib/class_list_macros.h>
PLUGINLIB_EXPORT_CLASS( hector_rviz_overlay_controls::ToolsOverlayDisplay, rviz::Display )
