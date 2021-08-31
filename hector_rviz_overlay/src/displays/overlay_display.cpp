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

#include "hector_rviz_overlay/displays/overlay_display.h"

#include "hector_rviz_overlay/overlay_manager.h"

#include <rviz/properties/float_property.h>
#include <rviz/properties/int_property.h>
#include <rviz/display_context.h>
#include <rviz/view_manager.h>

namespace hector_rviz_overlay
{

OverlayDisplay::OverlayDisplay() : OverlayDisplay( false ) { }

OverlayDisplay::OverlayDisplay( bool allow_multiple ) : overlay_( nullptr ), allow_multiple_( allow_multiple )
{
  z_index_property_ = new rviz::IntProperty( "Z-Index", 0,
                                             "The z-index of the overlay. In case of multiple overlays, the highest z-index will be drawn on top of the others.",
                                             this, SLOT( onZIndexChanged()));
  scale_property_ = new rviz::FloatProperty( "Scale", 1.0,
                                             "A scaling factor that is applied to the overlay so that people other than S. Kohlbrecher can see stuff on 4K monitors.",
                                             this, SLOT( onScaleChanged()));
  scale_property_->setMin( 0.1f );
  scale_property_->setMax( 10.0f );
}

OverlayDisplay::~OverlayDisplay()
{
  if ( overlay_ != nullptr )
  {
    OverlayManager::getSingleton().removeOverlay( overlay_ );
  }
}

void OverlayDisplay::onScaleChanged()
{
  if ( overlay_ == nullptr ) return;
  overlay_->setScale( scale_property_->getFloat());
}

void OverlayDisplay::onZIndexChanged()
{
  if ( overlay_ == nullptr ) return;
  overlay_->setZIndex( z_index_property_->getInt());
}

void OverlayDisplay::onInitialize()
{
  rviz::Display::onInitialize();
  OverlayManager::getSingleton().init( context_ );

  overlay_ = createOverlay();
  if ( OverlayManager::getSingleton().addOverlay( overlay_, allow_multiple_ ))
  {
    if ( !isEnabled())
    {
      overlay_->hide();
    }
    setStatus( rviz::StatusProperty::Ok, "Overlay", "Overlay created successfully." );
  }
  else
  {
    setStatus( rviz::StatusProperty::Error, "Overlay",
               "Could not create overlay. This overlay is single instance only!" );
  }
}

void OverlayDisplay::onEnable()
{
  rviz::Display::onEnable();
  if ( overlay_ == nullptr ) return;

  overlay_->show();
}

void OverlayDisplay::onDisable()
{
  rviz::Display::onDisable();
  if ( overlay_ == nullptr ) return;

  overlay_->hide();
}
}
