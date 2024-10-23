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

#ifndef HECTOR_RVIZ_OVERLAY_PATH_HELPER_H
#define HECTOR_RVIZ_OVERLAY_PATH_HELPER_H

#include <QString>

namespace hector_rviz_overlay
{

/*!
 * Resolves the given path which means if it starts with package://{package-name}/, the start is
 * substituted with the absolute path of the package. Otherwise the path is returned unchanged.
 * @param path The path.
 * @return The absolute path as described above.
 */
std::string resolvePath( const std::string &path );

/*!
 * Resolves the given path which means if it starts with package://{package-name}/, the start is
 * substituted with the absolute path of the package. Otherwise the path is returned unchanged.
 * @param path The path.
 * @return The absolute path as described above.
 */
QString resolvePath( const QString &path );
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_PATH_HELPER_H
