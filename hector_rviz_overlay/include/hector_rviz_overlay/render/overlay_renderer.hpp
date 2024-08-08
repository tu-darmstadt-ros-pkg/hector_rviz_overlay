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

#ifndef HECTOR_RVIZ_OVERLAY_OVERLAY_RENDERER_H
#define HECTOR_RVIZ_OVERLAY_OVERLAY_RENDERER_H

#include "hector_rviz_overlay/overlay.hpp"
#include "renderer.hpp"

#include <chrono>

namespace rviz_common
{
class DisplayContext;

class RenderPanel;
}

namespace hector_rviz_overlay
{

/*!
 * @class OverlayRenderer
 * @brief Responsible for rendering the overlays.
 *
 * This base class provides functionality to manage the rendered overlays and renders them in order (first to last).
 * See subclasses for specific rendering methods.
 */
class OverlayRenderer : public QObject, public Renderer
{
Q_OBJECT
private:
  static const unsigned int TimerHistoryLength = 16;
public:

  /*!
   * Creates an instance of an OverlayRenderer implementation depending on compile flags.
   * This method may cache the renderer or return a different instance on each call.
   * @param context A pointer to an rviz_common::DisplayContext.
   * @return An instance of a OverlayRenderer.
   */
  static OverlayRenderer *create( rviz_common::DisplayContext *context );

  explicit OverlayRenderer( rviz_common::DisplayContext *context );

  ~OverlayRenderer() override;

  /*!
   * Release the resources created in initialize().
   */
  virtual void releaseResources() = 0;

  QWindow *window() override;

  /*!
   * Adds the overlay to the collection of overlays that are rendered by this renderer and connects to the visibility
   * signal.
   * An overlay may always be handled by only one renderer.
   * The overlay is added to the end of the collection and therefore rendered last.
   * This method is implemented independently of insertOverlay.
   * @param overlay The overlay that is added.
   */
  void addOverlay( hector_rviz_overlay::OverlayPtr &overlay );

  /*!
   * Inserts the overlay into the collection of overlays that are rendered by this renderer and connects to the visibility
   * signal.
   * An overlay may always be handled by only one renderer.
   * The position in the collection of overlays determines when the overlay is rendered.
   * Elements at earlier positions are rendered before and elements at later positions are rendered after the element.
   * This method is implemented independently of addOverlay.
   * @param overlay The overlay that is inserted.
   * @param index The index in the collection of overlays where the overlay should be inserted.
   */
  void insertOverlay( hector_rviz_overlay::OverlayPtr &overlay, size_t index );

  /*!
   * Removes the overlay from the collection of overlays that are rendered by this renderer and disconnects the
   * visibility signal.
   * @param overlay The overlay that is removed.
   */
  void removeOverlay( hector_rviz_overlay::OverlayPtr &overlay );

  const std::vector<OverlayPtr> &overlays();

  std::vector<OverlayConstPtr> overlays() const;

  bool hasVisibleOverlay();

  bool isRendering() const;

protected slots:

  void onVisibilityChanged();

protected:

  /*!
   * Initializes all resources required for rendering.
   * You can expect that rviz_common's OpenGL context already exists at this point.
   */
  virtual void initialize() = 0;

  /*!
   * Prepare the given overlay for rendering. When this method is called you can expect the renderer to be initialized.
   * @param overlay The overlay that is prepared.
   */
  virtual void prepareOverlay( OverlayPtr &overlay );

  /*!
   * Will release the rendering resources for this overlay. This method is only called if prepareOverlay was called for
   * this overlay.
   * @param overlay The overlay that is release.
   */
  virtual void releaseOverlay( OverlayPtr &overlay );

  /*!
   * Updates the geometry of the overlays and renders them using the resources provided by subclasses.
   * Also handles measuring and drawing the frame time if enabled using the compile flag.
   */
  virtual void render();

  /*!
   * If the scene hasn't changed there is no need to render it again. Hence, this method should simply redraw the last
   * rendered scene.
   */
  virtual void redrawLastFrame() = 0;

  /*!
   * This method is called each time before the overlay's are rendered.
   * It should update required rendering resources with the potentially updated width and height of the overlay.
   * @param width The current width the rendered overlay should have.
   * @param height The current height the rendered overlay should have
   */
  virtual void prepareRender( int width, int height ) = 0;

  /*!
   * This method is called after the overlays are rendered.
   * It should handle cleaning up and making sure the content is visible.
   */
  virtual void finishRender() = 0;

  /*!
   * This method is called if all overlays are removed or invisible in which case remains of previous renderings should
   * be removed if necessary since the prepareRender and finishRender methods won't be called until an overlay is added
   * or visible again.
   */
  virtual void hide() = 0;

  rviz_common::DisplayContext *context_;
  rviz_common::RenderPanel *render_panel_;
  std::vector<OverlayPtr> overlays_;
  QRect geometry_;

  std::chrono::high_resolution_clock::time_point last_update_ = std::chrono::high_resolution_clock::time_point::max();
  int timer_index_;
  double timer_history_[TimerHistoryLength];
  double timer_average_;

  bool is_dirty_ = false;
  bool no_visible_overlays_ = false;
  bool initialized_ = false;
};
}

#endif //HECTOR_RVIZ_OVERLAY_OVERLAY_RENDERER_H
