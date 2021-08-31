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

#include <QAction>
#include <QButtonGroup>
#include <QGridLayout>
#include <QMenu>
#include <QPushButton>

#include <ros/package.h>

#include <rviz/new_object_dialog.h>
#include <rviz/tool.h>
#include <rviz/tool_manager.h>

namespace hector_rviz_overlay_controls
{

ToolsWidget::ToolsWidget( QWidget *parent )
  : QWidget( parent ), tool_manager_( nullptr ), tools_layout_( nullptr ), remove_tool_menu_( nullptr )
    , add_tool_button_( nullptr )
    , remove_tool_button_( nullptr )
{
  tool_button_group_ = new QButtonGroup( this );

  auto layout = new QHBoxLayout( this );
  layout->setContentsMargins(0, 0, 0, 0);
  tools_layout_ = new QHBoxLayout;
  layout->addLayout( tools_layout_ );

  QString package_path = QString::fromStdString( ros::package::getPath( "rviz" ));

  add_tool_button_ = new QPushButton( QIcon( package_path + "/icons/plus.png" ), "" );
  connect( add_tool_button_, SIGNAL( clicked()), this, SLOT( onAddToolClicked()));
  layout->addWidget( add_tool_button_ );

  remove_tool_button_ = new QPushButton( QIcon( package_path + "/icons/minus.png" ), "" );
  remove_tool_menu_ = new QMenu;
  connect( remove_tool_menu_, SIGNAL( triggered( QAction * )), this, SLOT( onRemoveToolMenuTriggered( QAction * )));
  remove_tool_button_->setMenu( remove_tool_menu_ );
  layout->addWidget( remove_tool_button_ );
}

void ToolsWidget::initializeTools( rviz::ToolManager *tool_manager )
{
  tool_manager_ = tool_manager;

  connect( tool_manager_, &rviz::ToolManager::toolAdded, this, &ToolsWidget::onToolAdded );
  connect( tool_manager_, &rviz::ToolManager::toolRemoved, this, &ToolsWidget::onToolRemoved );
  connect( tool_manager_, &rviz::ToolManager::toolChanged, this, &ToolsWidget::onToolChanged );

  for ( int i = 0; i < tool_manager_->numTools(); ++i )
  {
    rviz::Tool *tool = tool_manager_->getTool( i );
    addTool( tool );
  }
}

void ToolsWidget::setNamesVisible( bool value )
{
  names_visible_ = value;
  for ( int i = 0; i < tool_map_.keys().count(); ++i )
  {
    tool_map_.keys()[i]->setText( value ? tool_map_[tool_map_.keys()[i]]->getName() : "" );
  }
}

void ToolsWidget::setAddRemoveButtonsVisible( bool value )
{
  add_remove_buttons_visible_ = value;
  add_tool_button_->setVisible( value );
  remove_tool_button_->setVisible( value );
}

void ToolsWidget::onToolAdded( rviz::Tool *tool )
{
  addTool( tool );
}

void ToolsWidget::onToolChanged( rviz::Tool *tool )
{
  if ( tool != nullptr )
  {
    tool_buttons_[tool->getName()]->setChecked( true );
  }
}

void ToolsWidget::onToolRemoved( rviz::Tool *tool )
{
  removeTool( tool );
}

void ToolsWidget::onToolButtonChecked( bool checked )
{
  if ( !checked ) return;
  QPushButton *sender = static_cast<QPushButton *>(QObject::sender());
  if ( tool_manager_->getCurrentTool() == tool_map_[sender] ) return;
  tool_manager_->setCurrentTool( tool_map_[sender] );
}

void ToolsWidget::onAddToolClicked()
{
  QString class_id;
  rviz::NewObjectDialog *dialog = new rviz::NewObjectDialog( tool_manager_->getFactory(),
                                                             "Tool",
                                                             QStringList(),
                                                             tool_manager_->getToolClasses(),
                                                             &class_id );
  if ( dialog->exec() == QDialog::Accepted )
  {
    tool_manager_->addTool( class_id );
  }
}

void ToolsWidget::onRemoveToolMenuTriggered( QAction *action )
{
  for ( int i = 0; i < tool_manager_->numTools(); ++i )
  {
    if ( tool_manager_->getTool( i )->getName() != action->text()) continue;

    tool_manager_->removeTool( i );
    break;
  }
}

void ToolsWidget::addTool( rviz::Tool *tool )
{
  QPushButton *tool_button = new QPushButton( tool->getIcon(), names_visible_ ? tool->getName() : "" );
  tool_button->setToolTip( tool->getName());
  tool_button->setMouseTracking( true );
  tool_button->setCheckable( true );

  tool_buttons_[tool->getName()] = tool_button;
  tools_layout_->addWidget( tool_button );
  remove_tool_menu_->addAction( tool->getName());
  tool_map_[tool_button] = tool;
  tool_button_group_->addButton( tool_button );

  connect( tool_button, SIGNAL( toggled( bool )), this, SLOT( onToolButtonChecked( bool )));
}

void ToolsWidget::removeTool( rviz::Tool *tool )
{
  const QString &name = tool->getName();
  // Remove from the remove tool menu
  for ( int i = 0; i < remove_tool_menu_->actions().size(); ++i )
  {
    if ( remove_tool_menu_->actions()[i]->text() != name ) continue;

    remove_tool_menu_->removeAction( remove_tool_menu_->actions()[i] );
    break;
  }
  // Remove the button
  QPushButton *button = tool_buttons_[name];
  tools_layout_->removeWidget( button );
  tool_map_.remove( button );
  tool_button_group_->removeButton( button );
  tool_buttons_[name]->deleteLater();
  tool_buttons_.remove( name );
}

bool ToolsWidget::namesVisible() const
{
  return names_visible_;
}

bool ToolsWidget::addRemoveButtonsVisible() const
{
  return add_remove_buttons_visible_;
}
}
