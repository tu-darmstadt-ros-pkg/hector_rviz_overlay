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

#ifndef HECTOR_RVIZ_OVERLAY_QML_TOOL_MANAGER_H
#define HECTOR_RVIZ_OVERLAY_QML_TOOL_MANAGER_H

#include <QObject>
#include <rviz_common/tool.hpp>

namespace rviz_common
{
class ToolManager;
}

namespace hector_rviz_overlay
{

class QmlTool : public QObject
{
  Q_OBJECT
  // @formatter:off
  Q_PROPERTY( bool isSelected READ isSelected NOTIFY isSelectedChanged )
  Q_PROPERTY( QChar shortcutKey READ shortcutKey CONSTANT )
  Q_PROPERTY( bool accessAllKeys READ accessAllKeys CONSTANT )
  Q_PROPERTY( QString name READ name CONSTANT )
  Q_PROPERTY( QString description READ description CONSTANT )
  Q_PROPERTY( QString classId READ classId CONSTANT )
  Q_PROPERTY( QString iconSource READ iconSource CONSTANT )
  Q_PROPERTY( QObject *tool READ tool CONSTANT )
  // @formatter:on
public:
  explicit QmlTool( rviz_common::Tool *tool );

  bool isSelected() const;

  void setIsSelected( bool value );

  QChar shortcutKey() const;

  bool accessAllKeys() const;

  QString name() const;

  QString description() const;

  QString classId() const;

  QString iconSource() const;

  rviz_common::Tool *tool() { return tool_; }

signals:

  void isSelectedChanged();

private:
  rviz_common::Tool *tool_;
  bool is_selected_ = false;
};

class QmlToolManager : public QObject
{
  Q_OBJECT
  // @formatter:off
  Q_PROPERTY( QVariantList tools READ tools NOTIFY toolsChanged )
  Q_PROPERTY( QObject *currentTool READ currentTool WRITE setCurrentTool NOTIFY toolChanged )
  // @formatter:on
public:
  explicit QmlToolManager( rviz_common::ToolManager *tool_manager );

  QVariantList tools() const;

  Q_INVOKABLE QObject *addTool();

  Q_INVOKABLE QObject *addTool( const QString &class_lookup_name );

  Q_INVOKABLE QObject *getTool( const QString &class_id );

  Q_INVOKABLE void removeTool( int i );

  Q_INVOKABLE void removeTool( QObject *tool );

  Q_INVOKABLE void removeAll();

  QObject *currentTool();

  void setCurrentTool( QObject *tool );

signals:

  void toolAdded( QObject * );

  void toolRemoved( QObject * );

  void toolChanged( QObject * );

  void toolsChanged();

private slots:

  void disconnectSignals();

  void onToolAdded( rviz_common::Tool * );

  void onToolRemoved( rviz_common::Tool * );

  void onToolChanged( rviz_common::Tool * );

private:
  rviz_common::ToolManager *tool_manager_;
  QList<QmlTool *> tools_;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_QML_TOOL_MANAGER_H
