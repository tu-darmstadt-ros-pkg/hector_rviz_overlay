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

#ifndef HECTOR_RVIZ_OVERLAY_CONFIG_CONVERSION_H
#define HECTOR_RVIZ_OVERLAY_CONFIG_CONVERSION_H

#include <QVariant>
#include <rviz_common/config.hpp>

namespace hector_rviz_overlay
{

//! Converts a config to a QVariant. Maps become QVariantMap, lists QVariantList and values the
//! value they hold. Empty and invalid configs become an invalid QVariant.
QVariant configToVariant( const rviz_common::Config &config );

//! Writes the given map to the config, recursively creating map and list children as needed.
void writeToConfig( rviz_common::Config config, const QVariantMap &map );
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_CONFIG_CONVERSION_H
