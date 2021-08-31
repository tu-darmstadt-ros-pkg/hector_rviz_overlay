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

#include "hector_rviz_overlay/render/overlay_renderer.h"

#include "hector_rviz_overlay/render/glx_overlay_renderer.h"
#include "hector_rviz_overlay/render/image_overlay_renderer.h"

#include <chrono>

#include <QPainter>

#include <ros/ros.h>

#include <rviz/display_context.h>
#include <rviz/render_panel.h>
#include <rviz/view_manager.h>

namespace hector_rviz_overlay
{

OverlayRenderer *OverlayRenderer::create( rviz::DisplayContext *context )
{
#ifdef RENDER_OVERLAYS_USING_OPENGL
  OverlayRenderer *renderer = new GLXOverlayRenderer( context );
#else
  OverlayRenderer *renderer = new ImageOverlayRenderer( context );
#endif
  return renderer;
}

OverlayRenderer::OverlayRenderer( rviz::DisplayContext *context )
  : context_( context ), timer_index_( 0 ), no_visible_overlays_( true )
{
  render_panel_ = context_->getViewManager()->getRenderPanel();
  std::fill( timer_history_, timer_history_ + TimerHistoryLength, 0.0 );
  timer_average_ = 0;
}

OverlayRenderer::~OverlayRenderer()
{
}

void OverlayRenderer::addOverlay( OverlayPtr &overlay )
{
  overlays_.push_back( overlay );
  connect( overlay.get(), &Overlay::visibilityChanged, this, &OverlayRenderer::onVisibilityChanged,
           Qt::QueuedConnection );

  no_visible_overlays_ &= !overlay->isVisible();
  if ( !initialized_ ) return;
  prepareOverlay( overlay );
}

void OverlayRenderer::insertOverlay( hector_rviz_overlay::OverlayPtr &overlay, int index )
{
  if ( index >= overlays_.size())
  {
    addOverlay( overlay );
    return;
  }
  overlays_.insert( overlays_.begin() + index, overlay );
  connect( overlay.get(), &Overlay::visibilityChanged, this, &OverlayRenderer::onVisibilityChanged,
           Qt::QueuedConnection );

  no_visible_overlays_ &= !overlay->isVisible();
  if ( !initialized_ ) return;
  prepareOverlay( overlay );
}

void OverlayRenderer::removeOverlay( OverlayPtr &overlay )
{
  disconnect( overlay.get(), &Overlay::visibilityChanged, this, &OverlayRenderer::onVisibilityChanged );
  overlays_.erase( std::remove( overlays_.begin(), overlays_.end(), overlay ), overlays_.end());

  if ( initialized_ )
    releaseOverlay( overlay );

  if ( !no_visible_overlays_ && overlay->isVisible())
  {
    bool last_visible = true;
    for ( OverlayPtr &o : overlays_ )
    {
      if ( !o->isVisible()) continue;

      last_visible = false;
      break;
    }
    no_visible_overlays_ = last_visible;

    if ( no_visible_overlays_ && initialized_ )
    {
      hide();
    }
  }
}

void OverlayRenderer::onVisibilityChanged()
{
  auto sender = static_cast<Overlay *>(QObject::sender());
  if ( sender->isVisible())
  {
    // If the overlay is visible make sure it is drawn
    no_visible_overlays_ = false;
  }
  else
  {
    bool no_visible = true;
    for ( OverlayPtr &overlay : overlays_ )
    {
      if ( !overlay->isVisible()) continue;

      no_visible = false;
      break;
    }
    no_visible_overlays_ = no_visible;
    if ( no_visible_overlays_ && initialized_ )
    {
      hide();
    }
  }
}

bool OverlayRenderer::hasVisibleOverlay()
{
  return !no_visible_overlays_;
}

bool OverlayRenderer::isRendering() const
{
  return initialized_ && !no_visible_overlays_;
}

const std::vector<OverlayPtr> &OverlayRenderer::overlays()
{
  return overlays_;
}

std::vector<OverlayConstPtr> OverlayRenderer::overlays() const
{
  return std::vector<OverlayConstPtr>( overlays_.begin(), overlays_.end());
}

void OverlayRenderer::prepareOverlay( OverlayPtr &overlay )
{
  overlay->prepareRender( this );
}

void OverlayRenderer::releaseOverlay( OverlayPtr &overlay )
{
  overlay->releaseRenderResources();
}

void OverlayRenderer::render()
{
  if ( no_visible_overlays_ ) return;
  if ( !initialized_ )
  {
    initialize();
    initialized_ = true;
    for ( auto &overlay : overlays_ ) prepareOverlay( overlay );
  }

  auto start = std::chrono::high_resolution_clock::now();
  float delta = std::chrono::duration_cast<std::chrono::nanoseconds>( start - last_update_ ).count() / 1E9f;
  if ( delta < 0 ) delta = 0;
  last_update_ = start;
  for ( const auto &overlay : overlays_ )
  {
    overlay->update( delta );
  }

  bool is_dirty = false;

  int width = render_panel_->width();
  int height = render_panel_->height();

  // Have to compare size because geometry compares instances which would always create a new fbo
  if ( geometry_.size() != render_panel_->geometry().size())
  {
    geometry_ = render_panel_->geometry();
    is_dirty = true;
  }

  const QPoint &render_panel_top_left = render_panel_->mapToGlobal( QPoint( 0, 0 ));
  if ( geometry_.topLeft() != render_panel_top_left )
  {
    geometry_.moveTopLeft( render_panel_top_left );
    is_dirty = true;
  }
  else
  {
    for ( const auto &overlay : overlays_ )
    {
      if ( !overlay->isDirty()) continue;
      is_dirty = true;
      break;
    }
  }
  if ( !is_dirty )
  {
    redrawLastFrame();
    return;
  }

  prepareRender( width, height );
  // Draw the overlays
  for ( int i = 0; i < overlays_.size(); ++i )
  {
    OverlayPtr overlay = overlays_[i];
    if ( !overlay->isVisible()) continue;
    if ( overlay->geometry().topLeft() != geometry_.topLeft() || overlay->geometry().size() != geometry_.size())
    {
      overlays_[i]->setGeometry( geometry_ );
    }

    overlays_[i]->render( this );
  }

#ifdef DRAW_RENDERTIME
  QPainter painter( paintDevice());
  painter.setFont( QFont( "Arial", 16 ));
  painter.setPen( Qt::red );
  painter.drawText( 10, 80, QString( "Rendertime (ms): %1" ).arg( timer_average_ / TimerHistoryLength ));
  painter.end();
#endif

  finishRender();

  auto end = std::chrono::high_resolution_clock::now();
  timer_average_ -= timer_history_[timer_index_];
  timer_history_[timer_index_] = std::chrono::duration_cast<std::chrono::microseconds>( end - start ).count() / 1000.0;
  timer_average_ += timer_history_[timer_index_];
  timer_index_ = (timer_index_ + 1) % TimerHistoryLength;
}

QWindow *OverlayRenderer::window()
{
  return render_panel_->windowHandle();
}
}
