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

#include "ogre_gl_context.hpp"

#include <rviz_common/render_panel.hpp>
#include <rviz_common/view_manager.hpp>
#include <rviz_common/visualization_manager.hpp>
#include <rviz_rendering/render_window.hpp>

#include <OgreRenderTarget.h>
#include <OgreRenderTargetListener.h>
#include <OgreViewport.h>
#include <RenderSystems/GL/OgreGLContext.h>
#include <RenderSystems/GL/OgreGLRenderTarget.h>

#include <QCoreApplication>
#include <QQuickWindow>
#include <QThread>

namespace hector_rviz_overlay
{

void makeRenderTargetContextCurrent( Ogre::RenderTarget *target )
{
  if ( auto *gl_target = dynamic_cast<Ogre::GLRenderTarget *>( target ) )
    gl_target->getContext()->setCurrent();
}

class OgreGlContextGuard::Listener : public Ogre::RenderTargetListener
{
public:
  void preRenderTargetUpdate( const Ogre::RenderTargetEvent &evt ) override
  {
    Ogre::RenderTargetListener::preRenderTargetUpdate( evt );
    makeRenderTargetContextCurrent( evt.source );
  }
};

OgreGlContextGuard::OgreGlContextGuard( rviz_common::DisplayContext *context )
    : listener_( std::make_unique<Listener>() )
{
  render_panel_ = context->getViewManager()->getRenderPanel();
  rviz_rendering::RenderWindowOgreAdapter::addListener( render_panel_->getRenderWindow(),
                                                        listener_.get() );
  render_panel_destroyed_connection_ =
      QObject::connect( render_panel_, &QObject::destroyed, [this] { render_panel_ = nullptr; } );

  // Restoring at the start of the update cycle covers the whole cycle including the ROS callbacks:
  // the context can only have been stolen between event deliveries, never during the cycle.
  if ( auto *manager = qobject_cast<rviz_common::VisualizationManager *>( context ) ) {
    pre_update_connection_ = QObject::connect(
        manager, &rviz_common::VisualizationManager::preUpdate, [this] { restoreContext(); } );
  }
}

OgreGlContextGuard::~OgreGlContextGuard()
{
  QObject::disconnect( render_panel_destroyed_connection_ );
  QObject::disconnect( pre_update_connection_ );
  QObject::disconnect( frame_swapped_connection_ );
  if ( render_panel_ == nullptr )
    return;
  rviz_rendering::RenderWindowOgreAdapter::removeListener( render_panel_->getRenderWindow(),
                                                           listener_.get() );
}

void OgreGlContextGuard::restoreContext()
{
  if ( render_panel_ == nullptr )
    return;
  auto *viewport =
      rviz_rendering::RenderWindowOgreAdapter::getOgreViewport( render_panel_->getRenderWindow() );
  if ( viewport == nullptr )
    return;
  makeRenderTargetContextCurrent( viewport->getTarget() );
}

void OgreGlContextGuard::restoreContextAfterRenders( QQuickWindow *window )
{
  QObject::disconnect( frame_swapped_connection_ );
  frame_swapped_connection_ = QObject::connect(
      window, &QQuickWindow::frameSwapped, window,
      [this] {
        // With the threaded render loop the signal is emitted on the render thread, which has its
        // own current context, and only frames rendered on the GUI thread steal Ogre's.
        if ( QThread::currentThread() != QCoreApplication::instance()->thread() )
          return;
        restoreContext();
      },
      Qt::DirectConnection );
}
} // namespace hector_rviz_overlay
