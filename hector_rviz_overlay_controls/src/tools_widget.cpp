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

#include "hector_rviz_overlay_controls/tools_widget.h"

#include <QButtonGroup>
#include <QGridLayout>
#include <QMenu>
#include <QPushButton>

#include <rviz_common/tool.hpp>
#include <rviz_common/tool_manager.hpp>

namespace hector_rviz_overlay_controls
{

ToolsWidget::ToolsWidget( QWidget *parent ) : QWidget( parent )
{
  tool_button_group_ = new QButtonGroup( this );

  const auto layout = new QHBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  tools_layout_ = new QHBoxLayout;
  layout->addLayout( tools_layout_ );
}

void ToolsWidget::initializeTools( rviz_common::ToolManager *tool_manager )
{
  tool_manager_ = tool_manager;

  connect( tool_manager_, &rviz_common::ToolManager::toolAdded, this, &ToolsWidget::onToolAdded );
  connect( tool_manager_, &rviz_common::ToolManager::toolRemoved, this, &ToolsWidget::onToolRemoved );
  connect( tool_manager_, &rviz_common::ToolManager::toolChanged, this, &ToolsWidget::onToolChanged );

  for ( int i = 0; i < tool_manager_->numTools(); ++i ) {
    rviz_common::Tool *tool = tool_manager_->getTool( i );
    addTool( tool );
  }
  // Select currently active tool
  const auto *current_tool = tool_manager_->getCurrentTool();
  if ( current_tool == nullptr )
    return;
  tool_buttons_[current_tool->getName()]->setChecked( true );
}

void ToolsWidget::setNamesVisible( bool value )
{
  names_visible_ = value;
  const auto &keys = tool_map_.keys();
  for ( int i = 0; i < keys.count(); ++i ) {
    keys[i]->setText( value ? tool_map_[keys[i]]->getName() : "" );
  }
}

void ToolsWidget::onToolAdded( rviz_common::Tool *tool ) { addTool( tool ); }

void ToolsWidget::onToolChanged( rviz_common::Tool *tool )
{
  if ( tool != nullptr ) {
    tool_buttons_[tool->getName()]->setChecked( true );
  }
}

void ToolsWidget::onToolRemoved( rviz_common::Tool *tool ) { removeTool( tool ); }

void ToolsWidget::onToolButtonChecked( bool checked )
{
  if ( !checked )
    return;
  auto *sender = static_cast<QPushButton *>( QObject::sender() );
  if ( tool_manager_->getCurrentTool() == tool_map_[sender] )
    return;
  tool_manager_->setCurrentTool( tool_map_[sender] );
}

void ToolsWidget::addTool( rviz_common::Tool *tool )
{
  auto *tool_button = new QPushButton( tool->getIcon(), names_visible_ ? tool->getName() : "" );
  tool_button->setToolTip( tool->getName() );
  tool_button->setMouseTracking( true );
  tool_button->setCheckable( true );

  tool_buttons_[tool->getName()] = tool_button;
  tools_layout_->addWidget( tool_button );
  tool_map_[tool_button] = tool;
  tool_button_group_->addButton( tool_button );

  connect( tool_button, SIGNAL( toggled( bool ) ), this, SLOT( onToolButtonChecked( bool ) ) );
}

void ToolsWidget::removeTool( rviz_common::Tool *tool )
{
  const QString &name = tool->getName();
  // Remove the button
  QPushButton *button = tool_buttons_[name];
  tools_layout_->removeWidget( button );
  tool_map_.remove( button );
  tool_button_group_->removeButton( button );
  tool_buttons_[name]->deleteLater();
  tool_buttons_.remove( name );
}

bool ToolsWidget::namesVisible() const { return names_visible_; }
} // namespace hector_rviz_overlay_controls
