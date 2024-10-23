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

#ifndef HECTOR_RVIZ_OVERLAY_OVERLAY_MANAGER_H
#define HECTOR_RVIZ_OVERLAY_OVERLAY_MANAGER_H

#include "hector_rviz_overlay/popup/popup_overlay.hpp"
#include "hector_rviz_overlay/ui/ui_overlay.hpp"

#include <mutex>

#include <QImage>

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace rviz_common
{
class DisplayContext;
class RenderPanel;
} // namespace rviz_common

namespace hector_rviz_overlay
{
class OverlayRenderer;

/*!
 * @class OverlayManager
 * @brief This class is responsible for drawing and passing events to the overlays.
 *
 * The OverlayManager handles the two overlay classes UiOverlay and PopupOverlay.
 * While UI overlays are sorted by their z-index, the popup overlays are drawn newest on top and whenever a mouse event
 * is handled by a different popup overlay than the one at the top, the overlay that handled the event is moved to the top.
 */
class OverlayManager : public QObject
{
  Q_OBJECT

public:
  // This makes sure that we don't get copies accidentally.
  OverlayManager( OverlayManager const & ) = delete;

  void operator=( OverlayManager const & ) = delete;

  /*!
   * Returns a singleton instance of the OverlayManager which can be used to overlay Qt on top of the rviz_common::RenderPanel.
   *
   * @return A reference to the instance of the OverlayManager
   */
  static OverlayManager &getSingleton();

  //! @return The rviz_common::DisplayContext providing access to the main render panel and Ogre's SceneManager.
  rviz_common::DisplayContext *displayContext();

  /*!
   * Initializes the OverlayManager. Has to be called before issuing draw calls.
   * Does nothing if the OverlayManager is already initialized.
   * @param context An instance of a rviz_common::DisplayContext which provides access to the main render panel and Ogre's SceneManager
   */
  void init( rviz_common::DisplayContext *context );

  /*!
   * Adds the overlay to the collection of overlays managed by this class.
   * If an overlay with the same name already exists and \p unique_name_if_exists is false, the method fails.
   * If \p unique_name_if_exists is true, the overlay is renamed.
   * @param overlay The UI overlay that is added.
   * @param unique_name_if_exists Whether the overlay should be given a unique name if an overlay with the same name already exists.
   * @return True if successful, false if an overlay with the same name already exists and \p unique_name_if_exists is false
   */
  bool addOverlay( OverlayPtr overlay, bool unique_name_if_exists = false );

  /*!
   * Tries to get the overlay with the given name and calls the removeOverlay(OverlayPtr) overload.
   * @param name The name of the UI overlay.
   * @return True if the overlay with the given name was removed, false if it failed i.e. an overlay with the given name does not exist.
   */
  bool removeOverlay( const std::string &name );

  /*!
   * Removes the given UI overlay from the collection of UI overlays managed by this class.
   * @param overlay The UI overlay to be removed
   * @return True if removed, false if it wasn't found.
   */
  bool removeOverlay( OverlayPtr overlay );

  ///@{
  /*!
   * Tries to get the overlay object by its name.
   * @param name The name of the desired overlay.
   * @return The overlay if it was found, NULL otherwise
   */
  OverlayPtr getByName( const std::string &name );

  OverlayConstPtr getByName( const std::string &name ) const;
  ///@}

  /*!
   * Moves the given UI overlay to the front.
   * @param overlay The UI overlay that is moved to the front. Big surprise.
   */
  void moveToFront( UiOverlayPtr overlay );

  /*!
   * Moves the UI overlay with the given name to the front.
   * @throws std::invalid_argument if an UI overlay with the given name was not found.
   * @param name The name of the overlay that should be moved to the front.
   */
  void moveToFront( const std::string &name );

  /*!
   * Moves the given UI overlay to the back.
   * @param overlay The UI overlay that is moved to the back.
   */
  void moveToBack( UiOverlayPtr overlay );

  /*!
   * Moves the UI overlay with the given name to the back.
   * @throws std::invalid_argument if an UI overlay with the given name was not found.
   * @param name The name of the overlay that should be moved to the back.
   */
  void moveToBack( const std::string &name );

private slots:

  void onZIndexChanged();

  /*!
   * This slot removes the event filter and frees the rendering resources otherwise it would crash on exit because the
   * event filter receives events after the render panel is already destroyed and the renderer may hold a pointer to
   * the render panel that it might try to access when releasing resources.
   */
  void onAboutToQuit();

private:
  OverlayManager();

  ~OverlayManager() final;

  void checkEmpty();

  void insertOverlay( UiOverlayPtr overlay );

  bool eventFilter( QObject *receiver, QEvent *event ) final;

  bool handleMouseEvent( QObject *receiver, QMouseEvent *event );

  bool handleWheelEvent( QObject *receiver, QWheelEvent *event );

  bool handleKeyEvent( QObject *receiver, QKeyEvent *event );

  rviz_common::DisplayContext *context_;
  rviz_common::RenderPanel *render_panel_;
  std::vector<UiOverlayPtr> ui_overlays_;
  std::vector<PopupOverlayPtr> popup_overlays_;
  OverlayRenderer *renderer_;

  /*!
   * Stores the overlay that handled a mouse event (not down) previously, so that we can send events canceled if an
   * overlay with higher event priority handled the event e.g. to send HoverLeave events etc.
   */
  OverlayPtr mouse_overlay_;
  OverlayPtr mouse_down_overlay_;
  OverlayPtr focused_overlay_;
  bool sending_event_ = false;

  // Mutex for the access on overlays_
  mutable std::recursive_mutex mutex_;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_OVERLAY_MANAGER_H
