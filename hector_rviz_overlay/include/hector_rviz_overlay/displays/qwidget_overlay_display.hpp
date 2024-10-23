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

#ifndef HECTOR_RVIZ_OVERLAY_QWIDGET_OVERLAY_DISPLAY_H
#define HECTOR_RVIZ_OVERLAY_QWIDGET_OVERLAY_DISPLAY_H

#include "hector_rviz_overlay/ui/qwidget_overlay.hpp"
#include "overlay_display.hpp"

namespace rviz_common::properties
{
class EditableEnumProperty;
}

namespace hector_rviz_overlay
{

/*!
 * @class QWidgetOverlayDisplay
 * @brief A base class for QWidget overlay displays. Inherits from OverlayDisplay and adds a stylesheet property.
 *
 * Classes that inherit from this class only need to implement the onSetupUi( QWidget * ) method and add their Qt stuff
 * to the provided widget.
 */
class QWidgetOverlayDisplay : public OverlayDisplay
{
  Q_OBJECT
public:
  QWidgetOverlayDisplay();

  explicit QWidgetOverlayDisplay( bool allow_multiple );

protected slots:

  /*!
   * A Qt slot that is called whenever the style_sheet_property_ is changed.
   * The default functionality loads the stylesheet which can either be an absolute path, a package relative path or a style string.
   * A package relative path is specified using "package://{package_name}/{relative_path_in_package}".
   */
  virtual void updateStyleSheet();

protected:
  void onInitialize() override;

  UiOverlayPtr createOverlay() override;

  /*!
   * This method is called from onInitialize() once the Overlay was created and the Overlay's
   * QWidget can be used. If creating the Overlay fails, this method is never called.
   * @param widget The Overlay's QWidget which can also be accessed using Overlay::widget()
   */
  virtual void onSetupUi( QWidget *widget ) = 0;

  /*! A property that allows the user to apply a style sheet to an Overlay's top-level QWidget. */
  rviz_common::properties::EditableEnumProperty *style_sheet_property_;
  QWidgetOverlayPtr qwidget_overlay_;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_QWIDGET_OVERLAY_DISPLAY_H
