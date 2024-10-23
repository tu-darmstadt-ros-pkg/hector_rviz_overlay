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

#ifndef HECTOR_RVIZ_OVERLAY_OVERLAY_WIDGET_H
#define HECTOR_RVIZ_OVERLAY_OVERLAY_WIDGET_H

#include <QWidget>

namespace hector_rviz_overlay
{

/*!
 * @class OverlayWidget
 *
 * This class is a very simple subclass of QWidget that is used as the top-level widget for all overlays.
 */
class OverlayWidget : public QWidget
{
  Q_OBJECT
public:
  OverlayWidget();
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_OVERLAY_WIDGET_H
