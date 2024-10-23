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
#include <rviz_common/properties/property.hpp>

#include "hector_rviz_overlay/displays/overlay_display.hpp"

namespace hector_rviz_overlay
{

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
  rviz_common::properties::Property *property_;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_QML_RVIZ_PROPERTY_H
