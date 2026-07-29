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

#ifndef HECTOR_RVIZ_OVERLAY_OGRE_GL_CONTEXT_HPP
#define HECTOR_RVIZ_OVERLAY_OGRE_GL_CONTEXT_HPP

#include <QMetaObject>
#include <memory>

class QQuickWindow;

namespace Ogre
{
class RenderTarget;
}

namespace rviz_common
{
class DisplayContext;
class RenderPanel;
} // namespace rviz_common

namespace hector_rviz_overlay
{

//! Makes the OpenGL context the given render target renders into current.
//! Does nothing if the render system is not OpenGL.
void makeRenderTargetContextCurrent( Ogre::RenderTarget *target );

/*!
 * @class OgreGlContextGuard
 * @brief Keeps RViz's main render window rendering into its own OpenGL context.
 *
 * Ogre assumes that nothing else uses OpenGL on its thread and only rebinds its context when the
 * render target's context changed. Anything that makes another context current on the GUI thread
 * and leaves it current, e.g. Qt Quick when it falls back to the basic render loop with software
 * rendering, silently breaks Ogre: state such as the viewport after a resize is dropped and
 * resources such as the vertex buffers of a robot model are created in a context Ogre never
 * renders with, which makes them invisible.
 *
 * The guard restores Ogre's context before every render target update, which covers rendering,
 * and before every RViz update cycle, which covers the resources created from ROS callbacks and
 * display updates. For a QQuickWindow that renders on the GUI thread, additionally call
 * restoreContextAfterRenders() so the context is also restored immediately after every frame it
 * renders, which covers RViz calls made from QML in between update cycles.
 *
 * Create one of these for as long as a window that may render with OpenGL exists in RViz's window.
 */
class OgreGlContextGuard
{
public:
  explicit OgreGlContextGuard( rviz_common::DisplayContext *context );

  ~OgreGlContextGuard();

  OgreGlContextGuard( const OgreGlContextGuard & ) = delete;
  OgreGlContextGuard &operator=( const OgreGlContextGuard & ) = delete;

  //! Makes the context of RViz's main render window current.
  void restoreContext();

  /*!
   * Restores Ogre's context after every frame the given window renders on the GUI thread.
   *
   * With the threaded render loop the window renders with its own context on its own thread and
   * Ogre is unaffected; frames rendered on the GUI thread, e.g. with the basic render loop Qt
   * falls back to for software rendering, leave the window's context current there and are
   * followed by a restore. Replaces the window watched by a previous call.
   */
  void restoreContextAfterRenders( QQuickWindow *window );

private:
  class Listener;

  std::unique_ptr<Listener> listener_;
  rviz_common::RenderPanel *render_panel_ = nullptr;
  QMetaObject::Connection render_panel_destroyed_connection_;
  QMetaObject::Connection pre_update_connection_;
  QMetaObject::Connection frame_swapped_connection_;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_OGRE_GL_CONTEXT_HPP
