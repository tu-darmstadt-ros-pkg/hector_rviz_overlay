/*
 * Copyright (C) 2018  Stefan Fabian
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

#include "hector_rviz_overlay/path_helper.hpp"

#include <ament_index_cpp/get_package_share_directory.hpp>

namespace hector_rviz_overlay
{

std::string resolvePath( const std::string &path )
{
  if ( path.compare( 0, 10, "package://" ) == 0 ) {
    unsigned long pos_end_package_name = path.find( '/', 10 );
    std::string package = path.substr( 10, pos_end_package_name - 10 );
    return ament_index_cpp::get_package_share_directory( package ) +
           path.substr( pos_end_package_name );
  }
  return path;
}

QString resolvePath( const QString &path )
{
  if ( path.startsWith( "package://" ) ) {
    int pos_end_package_name = path.indexOf( '/', 10 );
    QString package = path.mid( 10, pos_end_package_name - 10 );
    auto package_path = QString::fromStdString(
        ament_index_cpp::get_package_share_directory( package.toStdString() ) );
    return package_path + path.mid( pos_end_package_name );
  }
  return path;
}
} // namespace hector_rviz_overlay
