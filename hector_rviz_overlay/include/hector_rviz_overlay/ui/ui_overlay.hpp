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

#ifndef HECTOR_RVIZ_OVERLAY_UI_OVERLAY_H
#define HECTOR_RVIZ_OVERLAY_UI_OVERLAY_H

#include "hector_rviz_overlay/overlay.hpp"

namespace hector_rviz_overlay
{

class UiOverlay : public Overlay
{
  Q_OBJECT
public:
  explicit UiOverlay( const std::string &name );

  ~UiOverlay() override;

  /*!
   * Overlays with a higher z-index are drawn on top of overlays with a lower z-index.
   * @return The z-index of this overlay instance.
   */
  int zIndex() const { return z_index_; }

  /*!
   * Sets the z-index for this overlay. Overlays with a higher z-index are drawn on top of overlays with a lower z-index.
   * @param value The new z-index for this overlay instance.
   */
  void setZIndex( int value );

signals:

  //! This signal is fired after the z-index is changed.
  void zIndexChanged();

protected:
  int z_index_;
};

typedef std::shared_ptr<UiOverlay> UiOverlayPtr;
typedef std::shared_ptr<const UiOverlay> UiOverlayConstPtr;
} // namespace hector_rviz_overlay

Q_DECLARE_METATYPE( hector_rviz_overlay::UiOverlayPtr );

#endif // HECTOR_RVIZ_OVERLAY_UI_OVERLAY_H
