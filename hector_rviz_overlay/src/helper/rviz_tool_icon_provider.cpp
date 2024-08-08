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

#include "hector_rviz_overlay/helper/rviz_tool_icon_provider.hpp"

#include <rviz_common/tool_manager.hpp>

namespace hector_rviz_overlay
{

RvizToolIconProvider::RvizToolIconProvider( rviz_common::ToolManager *tool_manager )
  : QQuickImageProvider( Pixmap ), tool_manager_( tool_manager ) { }

QPixmap RvizToolIconProvider::requestPixmap( const QString &id, QSize *size, const QSize &requestedSize )
{
  for ( int i = 0; i < tool_manager_->numTools(); ++i )
  {
    rviz_common::Tool *tool = tool_manager_->getTool( i );
    if ( tool->getClassId() != id ) continue;
    QList<QSize> sizes = tool->getIcon().availableSizes();
    QSize max_size( -1, -1 );
    for ( const auto &s : sizes )
    {
      if ( s.width() > max_size.width()) max_size = s;
    }
    if ( max_size.width() == -1 ) max_size = QSize( 48, 48 );
    *size = requestedSize;
    if ( size->width() == -1 )
    {
      if ( size->height() == -1 )
      {
        size->setHeight( max_size.width());
        size->setWidth( max_size.height());
      }
      else
      {
        size->setWidth( max_size.width() * size->height() / max_size.height());
      }
    }
    else if ( size->height() == -1 )
    {
      size->setHeight( max_size.height() * size->width() / max_size.width());
    }
    *size = tool->getIcon().actualSize( *size );
    return tool->getIcon().pixmap( *size );
  }
  return QQuickImageProvider::requestPixmap( id, size, requestedSize );
}
}
