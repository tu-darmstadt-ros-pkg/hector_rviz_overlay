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

#include "hector_rviz_overlay/overlay.hpp"

namespace hector_rviz_overlay
{

Overlay::Overlay( std::string name ) : name_( std::move( name ) )
{
  qRegisterMetaType<hector_rviz_overlay::OverlayPtr>();
}

Overlay::~Overlay() = default;

void Overlay::setGeometry( const QRect &value )
{
  geometry_ = value;
  requestRender();
}

void Overlay::setScale( float value )
{
  scale_ = value;
  requestRender();
}

void Overlay::setIsVisible( bool value )
{
  is_visible_ = value;
  emit visibilityChanged();
  requestRender();
}

void Overlay::requestRender() { is_dirty_ = true; }

void Overlay::render( Renderer *renderer )
{
  is_dirty_ = false;
  renderImpl( renderer );
}
} // namespace hector_rviz_overlay
