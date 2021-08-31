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

#ifndef HECTOR_RVIZ_OVERLAY_CONTROLS_TOOLS_WIDGET_H
#define HECTOR_RVIZ_OVERLAY_CONTROLS_TOOLS_WIDGET_H

#include <QWidget>

class QAction;
class QButtonGroup;
class QGridLayout;
class QHBoxLayout;
class QMenu;
class QPushButton;

namespace rviz
{
class Tool;
class ToolManager;
}

namespace hector_rviz_overlay_controls
{

class ToolsWidget : public QWidget
{
Q_OBJECT
  Q_PROPERTY( bool namesVisible
                READ
                namesVisible
                WRITE
                setNamesVisible )
  Q_PROPERTY( bool addRemoveButtonsVisible
                READ
                addRemoveButtonsVisible
                WRITE
                setAddRemoveButtonsVisible )
public:
  explicit ToolsWidget( QWidget *parent = nullptr );

  void initializeTools( rviz::ToolManager *tool_manager );

  bool namesVisible() const;

  void setNamesVisible( bool value );

  bool addRemoveButtonsVisible() const;

  void setAddRemoveButtonsVisible( bool value );

private slots:

  void onToolAdded( rviz::Tool *tool );

  void onToolRemoved( rviz::Tool *tool );

  void onToolChanged( rviz::Tool *tool );

  void onToolButtonChecked( bool checked );

  void onAddToolClicked();

  void onRemoveToolMenuTriggered( QAction *action );

private:

  void addTool( rviz::Tool *tool );

  void removeTool( rviz::Tool *tool );

  QHBoxLayout *tools_layout_;
  QHash<QString, QPushButton *> tool_buttons_;
  QHash<QPushButton *, rviz::Tool *> tool_map_;
  QButtonGroup *tool_button_group_;

  QPushButton *add_tool_button_;
  QPushButton *remove_tool_button_;
  QMenu *remove_tool_menu_;

  rviz::ToolManager *tool_manager_;

  bool names_visible_;
  bool add_remove_buttons_visible_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_CONTROLS_TOOLS_WIDGET_H
