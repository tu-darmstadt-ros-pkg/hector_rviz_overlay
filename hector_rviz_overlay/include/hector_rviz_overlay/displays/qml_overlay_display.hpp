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

#ifndef HECTOR_RVIZ_OVERLAY_QML_OVERLAY_DISPLAY_H
#define HECTOR_RVIZ_OVERLAY_QML_OVERLAY_DISPLAY_H

#include "overlay_display.hpp"

#include "hector_rviz_overlay/ui/qml_overlay.hpp"

namespace hector_rviz_overlay
{

/*!
 * @class QmlOverlayDisplay
 * @brief A base class for QML overlay displays. Inherits from OverlayDisplay.
 *
 * Classes that inherit from this class should implement the getPathToQml() method and return the
 * path to the qml file that should be loaded.
 *
 * Also, do not forget the Q_OBJECT macro, otherwise, your display may clash with other displays.
 */
class QmlOverlayDisplay : public OverlayDisplay
{
  Q_OBJECT
public:
  QmlOverlayDisplay();

  //! @param allow_multiple Whether or not to allow multiple instances of the display. Default: False (Single instance only)
  explicit QmlOverlayDisplay( bool allow_multiple );

  void load( const rviz_common::Config &config ) override;

  void save( rviz_common::Config config ) const override;

protected slots:

  virtual void onOverlayStatusChanged( QmlOverlay::Status status );

  void onOverlayContextCreated();

  /*!
   * The changed slot for the live reload property. Live reload means the qml file is watched for
   * changes and reloaded if it changed which is particularly useful for development.
   */
  virtual void onLiveReloadChanged();

protected:
  virtual QString getPathToQml();

  UiOverlayPtr createOverlay() override;

  QmlOverlayPtr qml_overlay_;

  rviz_common::Config overlay_config_;
  BoolProperty *live_reload_property_;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_QML_OVERLAY_DISPLAY_H
