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

#include "hector_rviz_overlay/displays/qwidget_overlay_display.hpp"
#include "hector_rviz_overlay/path_helper.hpp"

#include <QFile>
#include <QWidget>

#include <rviz_common/properties/editable_enum_property.hpp>

namespace hector_rviz_overlay
{

QWidgetOverlayDisplay::QWidgetOverlayDisplay() : QWidgetOverlayDisplay( false ) { }

QWidgetOverlayDisplay::QWidgetOverlayDisplay( bool allow_multiple )
    : OverlayDisplay( allow_multiple )
{
  style_sheet_property_ = new rviz_common::properties::EditableEnumProperty(
      "Style Sheet", "",
      "Sets the style sheet for the QWidget. "
      "Can be either a style sheet or a path to a style sheet. "
      "Use package://{package}/ to access package relative paths.",
      this, SLOT( updateStyleSheet() ) );
}

void QWidgetOverlayDisplay::onInitialize()
{
  OverlayDisplay::onInitialize();
  updateStyleSheet();

  onSetupUi( qwidget_overlay_->widget() );
}

UiOverlayPtr QWidgetOverlayDisplay::createOverlay()
{
  qwidget_overlay_ = std::make_shared<QWidgetOverlay>( metaObject()->className() );
  return qwidget_overlay_;
}

void QWidgetOverlayDisplay::updateStyleSheet()
{
  if ( overlay_ == nullptr )
    return;

  QString style_sheet = style_sheet_property_->getString();
  if ( style_sheet.endsWith( ".qss" ) ) {
    QFile file( resolvePath( style_sheet ) );
    if ( file.exists() ) {
      if ( file.open( QFile::ReadOnly ) ) {
        qwidget_overlay_->widget()->setStyleSheet( QLatin1String( file.readAll() ) );
        // For some reason the style isn't applied correctly without this if the setting was loaded from the rviz config
        // and therefore the stylesheet was set two times within a very short time frame before the widget is drawn
        qwidget_overlay_->widget()->ensurePolished();
        setStatus( rviz_common::properties::StatusProperty::Ok, "Style Sheet", "Style sheet loaded." );
      } else {
        setStatus( rviz_common::properties::StatusProperty::Error, "Style Sheet",
                   "Failed to open style sheet. Please check the path: " + style_sheet );
      }
    } else {
      setStatus( rviz_common::properties::StatusProperty::Error, "Style Sheet",
                 "Style sheet not found. Please check the path: " + style_sheet );
    }
  } else {
    qwidget_overlay_->widget()->setStyleSheet( style_sheet );
    qwidget_overlay_->widget()->ensurePolished();
    setStatus( rviz_common::properties::StatusProperty::Ok, "Style Sheet", "Style sheet applied." );
  }
}
} // namespace hector_rviz_overlay
