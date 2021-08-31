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

#ifndef HECTOR_RVIZ_OVERLAY_OVERLAY_H
#define HECTOR_RVIZ_OVERLAY_OVERLAY_H

#include <memory>

#include <QObject>
#include <QRect>


class QEvent;

namespace hector_rviz_overlay
{

class Renderer;

/*!
 * @class Overlay
 * @brief A base class for different kinds of overlays.
 *
 * Do NOT inherit from this class. Instead inherit from either UiOverlay or PopupOverlay because these are the two types
 * of overlays that are handled by the OverlayManager.
 *
 * @see OverlayManager
 */
class Overlay : public QObject
{
Q_OBJECT
public:
  /*!
   * The name passed here can differ from the name the overlay will have after it is passed to the OverlayManager if
   * when passing it to the OverlayManager it is specified that it should use a unique name if an overlay with the given
   * name already exists.
   *
   * @param name The name of the overlay.
   */
  explicit Overlay( std::string name );

  ~Overlay() override;

  /*!
   * This method is called before each render; regardless of whether the overlay is marked dirty or not and the overlay
   * can still be marked dirty by this method to require a render.
   * @param dt The time elapsed since the last update in seconds. (0 on the first call)
   */
  virtual void update( float dt ) {}

  /*!
   * This method is called once for the overlay to perform a one-time initialization before it is rendered.
   *
   * @param renderer An instance of the Renderer that is also used to render the overlay.
   */
  virtual void prepareRender( Renderer *renderer ) = 0;

  /*!
   * This method should release all resources that were created for rendering.
   */
  virtual void releaseRenderResources() = 0;

  /*!
   * Renders the overlay using the given renderer.
   *
   * @param renderer The renderer used to draw the overlay.
   */
  void render( Renderer *renderer );

  /*!
   * Handles events passed by the OverlayManager. Events with locations, e.g., MouseEvents will be adjusted so that the
   * local position is relative to the top left of the render panel (usually equivalent to the top left of the overlay).
   * The scale, however, is not treated by the OverlayManager and has to be handled by the overlays themselves.
   *
   * @param event The event that is passed
   * @return Whether the event was handled/consumed by the overlay or may propagate to the next overlay or rviz.
   */
  virtual bool handleEvent( QObject *receiver, QEvent *event ) = 0;

  /*!
   * This method is called whenever another overlay takes precedence over this overlay and ongoing events like mouse
   * hover events should be canceled i.e. by sending HoverLeave events.
   */
  virtual void handleEventsCanceled() = 0;

  /*!
   * <b>Note:</b>
   * The name that is returned isn't necessarily the name that was given to the OverlayManager to create this overlay.
   * @return The name of the overlay.
   */
  const std::string &name() const { return name_; }

  /*!
   * Updates the name of the overlay instance.
   * <b>Do NOT change the overlay's name after the OverlayManager knows about it.</b>
   * This method is usually used to generate a unique name if an overlay with this name already exists in the OverlayManager.
   * @param value The new name
   */
  void setName( const std::string &value ) { name_ = value; }

  /*!
   * Convenience function for setIsVisible( true ).
   */
  void show() { setIsVisible( true ); }

  /*!
   * Convenience function for setIsVisible( bool ).
   */
  void hide() { setIsVisible( false ); }

  /*!
   * This property allows to easily scale the content of an overlay i.e. for users with a high dpi monitor.
   * Default is 1.0.
   * @return The scaling factor of this overlay instance.
   */
  float scale() const { return scale_; }

  /*!
   * This property allows to easily scale the content of an overlay i.e. for users with a high dpi monitor.
   * @param value The new scaling factor for this overlay instance.
   */
  virtual void setScale( float value );

  /*!
   * If the scaling factor differs from 1.0, the geometry of the overlay will NOT be equal to the geometry of its content.
   * The geometry is updated by the OverlayManager and should always represent the global top-left, the width and the
   * height of the rviz::RenderPanel.
   * @return The geometry of this overlay.
   */
  const QRect &geometry() const { return geometry_; }

  /*!
   * This method is used by OverlayManager to update the overlay's geometry every time the rviz::RenderPanel's size changes.
   * If the scaling factor differs from 1.0, the geometry of the overlay will NOT be equal to the geometry of its content.
   * @param value The new geometry
   */
  virtual void setGeometry( const QRect &value );

  /*!
   * This property determines whether the overlay is drawn and responds to events or not.
   * @return True if this overlay is visible, false if not.
   */
  bool isVisible() const { return is_visible_; }

  /*!
   * This property determines whether the overlay is drawn and responds to events or not.
   * @param value Sets whether this overlay is visible or not.
   */
  void setIsVisible( bool value );

  /*!
   * This flag allows the renderer to save resources if the overlay does not require a redraw.
   *
   * When overriding this method make sure to factor in the value of the base implementation.
   * @return True if the overlay is dirty and needs to be rendered anew, false otherwise.
   */
  virtual bool isDirty() const { return is_dirty_; }

signals:

  /*!
   * This signal is fired after the visibility changed.
   */
  void visibilityChanged();

protected:

  /*!
   * This method has to be implemented in subclasses.
   * It should use the given Renderer to render its contents according to the geometry of the widget.
   * When using the geometry keep the scale in mind which is not accounted for in the geometry property.
   *
   * @param renderer The renderer used to draw the overlay.
   */
  virtual void renderImpl( Renderer *renderer ) = 0;

  //! Marks the overlay as dirty to make sure it is re-rendered in the next render
  void requestRender();

private:
  std::string name_;
  QRect geometry_;
  float scale_ = 1.0f;
  bool is_visible_ = true;
  bool is_dirty_ = true;
};

typedef std::shared_ptr<Overlay> OverlayPtr;
typedef std::shared_ptr<const Overlay> OverlayConstPtr;
}

Q_DECLARE_METATYPE( hector_rviz_overlay::OverlayPtr );

#endif //HECTOR_RVIZ_OVERLAY_OVERLAY_H
