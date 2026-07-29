/*
 * Copyright (C) 2025  Stefan Fabian
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

#include "config_conversion.hpp"

namespace hector_rviz_overlay
{

QVariant configToVariant( const rviz_common::Config &config )
{
  switch ( config.getType() ) {
  case rviz_common::Config::Map: {
    QVariantMap result;
    rviz_common::Config::MapIterator it = config.mapIterator();
    while ( it.isValid() ) {
      result.insert( it.currentKey(), configToVariant( it.currentChild() ) );
      it.advance();
    }
    return result;
  }
  case rviz_common::Config::List: {
    QVariantList result;
    for ( int i = 0; i < config.listLength(); ++i ) {
      result.append( configToVariant( config.listChildAt( i ) ) );
    }
    return result;
  }
  case rviz_common::Config::Value:
    return config.getValue();
  case rviz_common::Config::Empty:
  case rviz_common::Config::Invalid:
    return {};
  }
  return {};
}

namespace
{
// Kept internal because a QVariant that holds a map or a list has to be written as a map or list
// child and not as a single value.
void writeMapToConfig( rviz_common::Config config, const QVariantMap &map );

void writeListToConfig( rviz_common::Config config, const QVariantList &list );

void writeVariantToConfig( rviz_common::Config config, const QVariant &variant )
{
  switch ( variant.type() ) {
  case QVariant::Map:
    writeMapToConfig( config, variant.toMap() );
    break;
  case QVariant::List:
    writeListToConfig( config, variant.toList() );
    break;
  default:
    config.setValue( variant );
  }
}

void writeMapToConfig( rviz_common::Config config, const QVariantMap &map )
{
  for ( const auto &key : map.keys() ) {
    writeVariantToConfig( config.mapMakeChild( key ), map[key] );
  }
}

void writeListToConfig( rviz_common::Config config, const QVariantList &list )
{
  for ( const auto &item : list ) { writeVariantToConfig( config.listAppendNew(), item ); }
}
} // namespace

void writeToConfig( rviz_common::Config config, const QVariantMap &map )
{
  writeMapToConfig( config, map );
}
} // namespace hector_rviz_overlay
