/*
 * Copyright (C) 2020  Stefan Fabian
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

#include "hector_rviz_overlay/helper/qml_tool_manager.h"

#include <rviz/new_object_dialog.h>
#include <rviz/tool_manager.h>

namespace hector_rviz_overlay
{

QmlTool::QmlTool( rviz::Tool *tool ) : tool_( tool ) { }

bool QmlTool::isSelected() const { return is_selected_; }

void QmlTool::setIsSelected( bool value )
{
  if ( is_selected_ == value ) return;
  is_selected_ = value;
  emit isSelectedChanged();
}

QChar QmlTool::shortcutKey() const { return tool_->getShortcutKey(); }

bool QmlTool::accessAllKeys() const { return tool_->accessAllKeys(); }

QString QmlTool::name() const { return tool_->getName(); }

QString QmlTool::description() const { return tool_->getDescription(); }

QString QmlTool::classId() const { return tool_->getClassId(); }

QString QmlTool::iconSource() const { return "image://rviz_tool_icons/" + classId(); }


// ======================== Tool Manager ========================

QmlToolManager::QmlToolManager( rviz::ToolManager *tool_manager ) : tool_manager_( tool_manager )
{
  connect( tool_manager_, &rviz::ToolManager::toolAdded, this, &QmlToolManager::onToolAdded );
  connect( tool_manager_, &rviz::ToolManager::toolRemoved, this, &QmlToolManager::onToolRemoved );
  connect( tool_manager_, &rviz::ToolManager::toolChanged, this, &QmlToolManager::onToolChanged );

  for ( int i = 0; i < tool_manager_->numTools(); ++i )
  {
    auto *tool = new QmlTool( tool_manager_->getTool( i ));
    if ( tool->tool() == tool_manager_->getCurrentTool()) tool->setIsSelected( true );
    tools_.append( tool );
  }
}

QVariantList QmlToolManager::tools() const
{
  QVariantList result;
  result.reserve( tools_.size());
  for ( const auto &tool : tools_ ) result.append( QVariant::fromValue<QObject *>( tool ));
  return result;
}

void QmlToolManager::onToolAdded( rviz::Tool *tool )
{
  QmlTool *result;
  tools_.append( result = new QmlTool( tool ));
  emit toolAdded( result );
  emit toolsChanged();
}

void QmlToolManager::onToolRemoved( rviz::Tool *tool )
{
  for ( int i = 0; i < tools_.size(); ++i )
  {
    if ( tools_[i]->tool() != tool ) continue;
    emit toolRemoved( tools_[i] );
    delete tools_[i];
    tools_.removeAt( i );
    break;
  }
  emit toolsChanged();
}

void QmlToolManager::onToolChanged( rviz::Tool *tool )
{
  for ( auto &qml_tool : tools_ )
  {
    qml_tool->setIsSelected( qml_tool->tool() == tool );
    if ( qml_tool->tool() == tool ) emit toolChanged( qml_tool );
  }
}

QObject *QmlToolManager::addTool()
{
  QString class_id;
  auto *dialog = new rviz::NewObjectDialog( tool_manager_->getFactory(), "Tool",
                                            QStringList(), tool_manager_->getToolClasses(),
                                            &class_id );
  if ( dialog->exec() == QDialog::Accepted )
  {
    return addTool( class_id );
  }
  return nullptr;
}

QObject *QmlToolManager::addTool( const QString &class_lookup_name )
{
  rviz::Tool *tool = tool_manager_->addTool( class_lookup_name );
  assert( !tools_.empty());
  assert( tools_.last()->tool() == tool );
  return tools_.last();
}

void QmlToolManager::removeTool( int i )
{
  if ( tools_.size() <= i ) return;
  assert( tools_.at( i )->tool() == tool_manager_->getTool( i ));
  tools_.removeAt( i );
  tool_manager_->removeTool( i );
}

void QmlToolManager::removeTool( QObject *tool )
{
  for ( int i = 0; i < tools_.size(); ++i )
  {
    if ( tools_[i] != tool ) continue;
    removeTool( i );
    break;
  }
}

void QmlToolManager::removeAll()
{
  tool_manager_->removeAll();
}

QObject *QmlToolManager::currentTool()
{
  rviz::Tool *current = tool_manager_->getCurrentTool();
  for ( const auto &tool : tools_ )
  {
    if ( tool->tool() == current ) return tool;
  }
  return nullptr;
}

void QmlToolManager::setCurrentTool( QObject *qtool )
{
  auto *tool = qobject_cast<QmlTool *>( qtool );
  if ( tool == nullptr ) return;
  tool_manager_->setCurrentTool( tool->tool());
}
}
