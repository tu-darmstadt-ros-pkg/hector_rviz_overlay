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

#ifndef HECTOR_RVIZ_OVERLAY_RVIZ_TOOL_ICON_PROVIDER_H
#define HECTOR_RVIZ_OVERLAY_RVIZ_TOOL_ICON_PROVIDER_H

#include <QQuickImageProvider>

namespace rviz_common
{
class ToolManager;
}

namespace hector_rviz_overlay
{

class RvizToolIconProvider : public QQuickImageProvider
{
public:
  explicit RvizToolIconProvider(rviz_common::ToolManager *tool_manager);

  QPixmap requestPixmap( const QString &id, QSize *size, const QSize &requestedSize ) override;

private:
  rviz_common::ToolManager *tool_manager_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_RVIZ_TOOL_ICON_PROVIDER_H
