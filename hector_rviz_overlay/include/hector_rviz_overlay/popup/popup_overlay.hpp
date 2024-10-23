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

#ifndef HECTOR_RVIZ_OVERLAY_POPUP_OVERLAY_H
#define HECTOR_RVIZ_OVERLAY_POPUP_OVERLAY_H

#include "hector_rviz_overlay/overlay.hpp"

namespace hector_rviz_overlay
{

/*!
 * A base class for popup overlays.
 * Popup overlays are handled a little different than UiOverlays by the OverlayManager.
 * While the drawing and event order of UiOverlays is determined by their z-order.
 * The order of PopupOverlays depends on the last interaction.
 * When it is added, the PopupOverlay is added to the top - meaning it's drawn on top of everything
 * else - if the PopupOverlay that is on top does not handle a mouse event but another one does, the
 * popup handling the event is moved to the top.
 */
class PopupOverlay : public Overlay
{
  Q_OBJECT
public:
  explicit PopupOverlay( const std::string &name );

  ~PopupOverlay() override;
};

typedef std::shared_ptr<PopupOverlay> PopupOverlayPtr;
typedef std::shared_ptr<const PopupOverlay> PopupOverlayConstPtr;
} // namespace hector_rviz_overlay

Q_DECLARE_METATYPE( hector_rviz_overlay::PopupOverlayPtr );

#endif // HECTOR_RVIZ_OVERLAY_POPUP_OVERLAY_H
