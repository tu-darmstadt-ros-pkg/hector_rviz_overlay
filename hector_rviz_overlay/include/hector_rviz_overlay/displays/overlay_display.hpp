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

#ifndef HECTOR_RVIZ_OVERLAY_OVERLAY_DISPLAY_H
#define HECTOR_RVIZ_OVERLAY_OVERLAY_DISPLAY_H

#include "hector_rviz_overlay/ui/ui_overlay.hpp"

#include <rviz_common/display.hpp>

namespace rviz_common::properties
{
class FloatProperty;
class IntProperty;
} // namespace rviz_common::properties

namespace hector_rviz_overlay
{

/*!
 * @class OverlayDisplay
 * @brief A base class for all overlay displays.
 * This class provides the boilerplate code that is required to manage an overlay including a z-index and scale property.
 */
class OverlayDisplay : public rviz_common::Display
{
  Q_OBJECT
public:
  /*!
   * Creates an instance of OverlayDisplay that does not allow multiple instances.
   */
  OverlayDisplay();

  /*!
   * Creates an instance of OverlayDisplay
   * @param allow_multiple Sets whether multiple instances of this OverlayDisplay can be added to rviz.
   */
  explicit OverlayDisplay( bool allow_multiple );

  ~OverlayDisplay() override;

protected slots:

  /*!
   * A Qt slot that is called whenever the scale_property_ is changed.
   * The default implementation simply forwards the new scale to the overlay_ instance.
   */
  virtual void onScaleChanged();

  /*!
   * A Qt slot that is called whenever the z_index_property_ is changed.
   * The default implementation simply forwards the new z-index value to the overlay_ instance.
   */
  virtual void onZIndexChanged();

protected:
  /*!
   * Creates the Overlay instance using createOverlay() and tries to add it to the OverlayManager.
   * @see rviz_common::Display::onInitialize()
   */
  void onInitialize() override;

  /*!
   * This method is called in the onInitialize() method to create the overlay.
   * @return An instantiation of the concrete overlay.
   */
  virtual UiOverlayPtr createOverlay() = 0;

  /*!
   * Shows the Overlay.
   * @see rviz_common::Display::onEnable()
   */
  void onEnable() override;

  /*!
   * Hides the Overlay.
   * @see rviz_common::Display::onDisable()
   */
  void onDisable() override;

  /*! A property that allows the user to change the z-index of the Overlay. */
  rviz_common::properties::IntProperty *z_index_property_;
  /*! A property that allows the user to change the scale of the Overlay. */
  rviz_common::properties::FloatProperty *scale_property_;
  /*! A pointer to an instance of Overlay which is created in onInitialize(). */
  UiOverlayPtr overlay_;

private:
  /*! A boolean indicating whether multiple instances of this overlay are allowed */
  bool allow_multiple_;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_OVERLAY_DISPLAY_H
