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

#ifndef HECTOR_RVIZ_OVERLAY_CENTER_TRACKER_H
#define HECTOR_RVIZ_OVERLAY_CENTER_TRACKER_H

#include "hector_rviz_overlay/popup/positioning/point_tracker.hpp"

namespace hector_rviz_overlay
{

class CenterTracker : public PointTracker
{
public:
  QPoint getPoint( int width, int height ) override;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_CENTER_TRACKER_H
