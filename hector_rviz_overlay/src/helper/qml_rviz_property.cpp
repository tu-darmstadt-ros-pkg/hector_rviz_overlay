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

QmlRvizProperty::QmlRvizProperty( rviz_common::properties::Property *property ) : property_( property )
{
  connect( property_, &rviz_common::properties::Property::changed, this, &QmlRvizProperty::onPropertyChanged );
}

QVariant QmlRvizProperty::value() const
{
  return property_->getValue();
}

void QmlRvizProperty::setValue( const QVariant &value )
{
  property_->setValue( value );
}

rviz_common::properties::Property *QmlRvizProperty::property()
{
  return property_;
}

void QmlRvizProperty::onPropertyChanged()
{
  emit valueChanged( property_->getValue());
}
}
