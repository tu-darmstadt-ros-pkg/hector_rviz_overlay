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

#include "hector_rviz_overlay/helper/qml_rviz_property.hpp"

namespace hector_rviz_overlay
{

rviz_common::properties::Property *findChildProperty( rviz_common::properties::Property *property,
                                                      const QString &path )
{
  if ( property == nullptr )
    return nullptr;
  for ( int i = 0; i < property->numChildren(); ++i ) {
    if ( property->childAt( i )->getName() != path )
      continue;
    return property->childAt( i );
  }
  return nullptr;
}

bool setPropertyValueCoerced( rviz_common::properties::Property *property, const QVariant &value )
{
  if (property == nullptr)
    return false;
  const QVariant current = property->getValue();
  // Only coerce when the property has a typed value to match. Valueless properties (e.g. group
  // headers) carry an invalid variant, so pass the value through unchanged as rviz would.
  if ( current.isValid() && value.userType() != current.userType() ) {
    QVariant coerced = value;
    if ( !coerced.convert( current.userType() ) )
      return false;
    property->setValue( coerced );
    return true;
  }
  property->setValue( value );
  return true;
}

QmlRvizProperty::QmlRvizProperty( rviz_common::properties::Property *property )
    : property_( property )
{
  connect( property_, &rviz_common::properties::Property::changed, this,
           &QmlRvizProperty::onPropertyChanged );
}

QVariant QmlRvizProperty::value() const
{
  if ( property_.isNull() )
    return {};
  return property_->getValue();
}

void QmlRvizProperty::setValue( const QVariant &value )
{
  if ( property_.isNull() )
    return;
  setPropertyValueCoerced( property_, value );
}

rviz_common::properties::Property *QmlRvizProperty::property() { return property_; }

void QmlRvizProperty::onPropertyChanged()
{
  if ( property_.isNull() )
    return;
  emit valueChanged( property_->getValue() );
}
} // namespace hector_rviz_overlay
