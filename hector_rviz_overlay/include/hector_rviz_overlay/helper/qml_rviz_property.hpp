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

#ifndef HECTOR_RVIZ_OVERLAY_QML_RVIZ_PROPERTY_H
#define HECTOR_RVIZ_OVERLAY_QML_RVIZ_PROPERTY_H

#include <QObject>
#include <QPointer>
#include <rviz_common/properties/property.hpp>

#include "hector_rviz_overlay/displays/overlay_display.hpp"

namespace hector_rviz_overlay
{

/*!
 * Finds the direct child property of the given property whose name matches path.
 * Unlike Property::subProp this does not log and return a FailureProperty on a miss.
 * @return The matching child property or nullptr if none matches.
 */
rviz_common::properties::Property *findChildProperty( rviz_common::properties::Property *property,
                                                      const QString &path );

/*!
 * Sets a property's value, coercing the incoming variant to the property's current value type first.
 * rviz's Property::setValue stores the variant verbatim, and its editor delegate picks the widget
 * from the stored type, so assigning e.g. the string "true" to a BoolProperty replaces the bool and
 * turns the checkbox into a text field. Coercion keeps the type-specific editor intact.
 * @return false if the value cannot be converted to the property's type; true otherwise.
 */
bool setPropertyValueCoerced( rviz_common::properties::Property *property, const QVariant &value );

class QmlRvizProperty : public QObject
{
  Q_OBJECT
  // @formatter:off
  Q_PROPERTY( QVariant value READ value WRITE setValue NOTIFY valueChanged )
  // @formatter:on
public:
  explicit QmlRvizProperty( rviz_common::properties::Property *property );

  QVariant value() const;

  void setValue( const QVariant &value );

  rviz_common::properties::Property *property();

signals:

  void valueChanged( const QVariant &value );

private slots:
  void onPropertyChanged();

private:
  QPointer<rviz_common::properties::Property> property_;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_QML_RVIZ_PROPERTY_H
