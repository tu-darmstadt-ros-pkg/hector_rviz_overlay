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

#include "helper/config_conversion.hpp"

#include <gtest/gtest.h>

using hector_rviz_overlay::configToVariant;
using hector_rviz_overlay::writeToConfig;

namespace
{
QVariantMap makeNestedMap()
{
  QVariantMap inner;
  inner.insert( "flag", true );
  inner.insert( "count", 42 );

  QVariantList list;
  list.append( 1.5 );
  list.append( QString( "text" ) );
  list.append( inner );

  QVariantMap root;
  root.insert( "list", list );
  root.insert( "nested", inner );
  root.insert( "name", QString( "root" ) );
  return root;
}
} // namespace

TEST( ConfigConversion, roundTrip )
{
  const QVariantMap original = makeNestedMap();

  rviz_common::Config config;
  writeToConfig( config, original );

  const QVariant read_back = configToVariant( config );
  ASSERT_EQ( read_back.type(), QVariant::Map );

  rviz_common::Config second;
  writeToConfig( second, read_back.toMap() );
  EXPECT_EQ( configToVariant( second ), read_back );

  // Values pass through the config as strings, hence, compare the string representations.
  const QVariantMap result = read_back.toMap();
  EXPECT_EQ( result["name"].toString(), original["name"].toString() );
  ASSERT_EQ( result["nested"].type(), QVariant::Map );
  EXPECT_EQ( result["nested"].toMap()["flag"].toBool(), true );
  EXPECT_EQ( result["nested"].toMap()["count"].toInt(), 42 );
  ASSERT_EQ( result["list"].type(), QVariant::List );
  const QVariantList result_list = result["list"].toList();
  ASSERT_EQ( result_list.size(), 3 );
  EXPECT_DOUBLE_EQ( result_list[0].toDouble(), 1.5 );
  EXPECT_EQ( result_list[1].toString(), QString( "text" ) );
  ASSERT_EQ( result_list[2].type(), QVariant::Map );
  EXPECT_EQ( result_list[2].toMap()["count"].toInt(), 42 );
}

TEST( ConfigConversion, nodeTypes )
{
  rviz_common::Config config;
  writeToConfig( config, makeNestedMap() );

  ASSERT_EQ( config.getType(), rviz_common::Config::Map );
  EXPECT_EQ( config.mapGetChild( "name" ).getType(), rviz_common::Config::Value );
  EXPECT_EQ( config.mapGetChild( "nested" ).getType(), rviz_common::Config::Map );
  EXPECT_EQ( config.mapGetChild( "nested" ).mapGetChild( "flag" ).getType(),
             rviz_common::Config::Value );

  const rviz_common::Config list = config.mapGetChild( "list" );
  ASSERT_EQ( list.getType(), rviz_common::Config::List );
  ASSERT_EQ( list.listLength(), 3 );
  EXPECT_EQ( list.listChildAt( 0 ).getType(), rviz_common::Config::Value );
  EXPECT_EQ( list.listChildAt( 2 ).getType(), rviz_common::Config::Map );
  EXPECT_EQ( list.listChildAt( 2 ).mapGetChild( "count" ).getType(), rviz_common::Config::Value );
}

TEST( ConfigConversion, invalidConfigYieldsInvalidVariant )
{
  EXPECT_FALSE( configToVariant( rviz_common::Config() ).isValid() );
}
