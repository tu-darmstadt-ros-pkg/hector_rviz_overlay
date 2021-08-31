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

#include "hector_rviz_overlay/overlay_manager.h"
#include "hector_rviz_overlay/render/overlay_renderer.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#include <Ogre.h>

#include <rviz/display_context.h>
#include <rviz/render_panel.h>
#include <rviz/view_manager.h>

#include <ros/ros.h>

namespace hector_rviz_overlay
{

OverlayManager &OverlayManager::getSingleton()
{
  static OverlayManager instance;
  return instance;
}

OverlayManager::OverlayManager()
  : context_( nullptr )
    , render_panel_( nullptr )
    , renderer_( nullptr )
    , mouse_overlay_( nullptr )
    , mouse_down_overlay_( nullptr )
    , focused_overlay_( nullptr )
{
}

OverlayManager::~OverlayManager() = default;

rviz::DisplayContext *OverlayManager::displayContext()
{
  return context_;
}

void OverlayManager::init( rviz::DisplayContext *context )
{
  if ( renderer_ != nullptr ) return;

  context_ = context;
  render_panel_ = context_->getViewManager()->getRenderPanel();
  connect( QCoreApplication::instance(), &QCoreApplication::aboutToQuit, this, &OverlayManager::onAboutToQuit,
           Qt::DirectConnection );
}

void OverlayManager::onAboutToQuit()
{
  qApp->removeEventFilter( this );
  if ( renderer_ == nullptr ) return;
  // Has to be released at this point because, e.g., the GLXOverlayRenderer requires its pointer to the RenderPanel to be valid
  renderer_->releaseResources();
  delete renderer_;
  renderer_ = nullptr;
}

bool OverlayManager::addOverlay( OverlayPtr overlay, bool unique_name_if_exists )
{
  OverlayPtr overlay_by_name = getByName( overlay->name());
  if ( overlay_by_name != nullptr )
  {
    if ( overlay_by_name == overlay ) return false;

    if ( !unique_name_if_exists )
    {
      return false;
    }
    int counter = 1;
    while ( getByName( overlay->name() + std::to_string( counter )) != nullptr ) counter++;
    overlay->setName( overlay->name() + std::to_string( counter ));
  }

  if ( renderer_ == nullptr )
  {
    renderer_ = OverlayRenderer::create( context_ );
    qApp->installEventFilter( this );
  }

  UiOverlayPtr ui_overlay = std::dynamic_pointer_cast<UiOverlay>( overlay );
  if ( ui_overlay != nullptr )
  {
    insertOverlay( ui_overlay );

    connect( ui_overlay.get(), &UiOverlay::zIndexChanged, this, &OverlayManager::onZIndexChanged );
    return true;
  }
  PopupOverlayPtr popup_overlay = std::dynamic_pointer_cast<PopupOverlay>( overlay );
  if ( popup_overlay != nullptr )
  {
    popup_overlays_.push_back( popup_overlay );
    renderer_->addOverlay( overlay );
    return true;
  }
  ROS_ERROR_NAMED( "OverlayManager", "Tried to add unknown type of overlay!" );
  return false;
}

bool OverlayManager::removeOverlay( const std::string &name )
{
  return removeOverlay( getByName( name ));
}

void OverlayManager::checkEmpty()
{
  // If there are no overlays left, the renderer is destroyed.
  // This is done for two reasons: The first and most obvious, we don't need a renderer if we have no overlays
  // and second, when deleting it in OverlayManager's destructor it crashes because of a multithreading issue
  // and deleting when the RenderPanel is destroyed doesn't work either because the overlays are removed after that
  // which leads to a different crash.
  if ( ui_overlays_.empty() && popup_overlays_.empty())
  {
    qApp->removeEventFilter( this );
    if ( renderer_ != nullptr )
    {
      renderer_->releaseResources();
      delete renderer_;
      renderer_ = nullptr;
    }
  }
}

bool OverlayManager::removeOverlay( OverlayPtr overlay )
{
  if ( overlay == nullptr ) return false;
  if ( overlay == mouse_overlay_ )
  {
    overlay->handleEventsCanceled();
    mouse_overlay_ = nullptr;
  }

  if ( overlay == mouse_down_overlay_ )
  {
    overlay->handleEventsCanceled();
    mouse_down_overlay_ = nullptr;
  }

  if ( overlay == focused_overlay_ )
  {
    overlay->handleEventsCanceled();
    focused_overlay_ = nullptr;
  }

  for ( auto it = ui_overlays_.begin(); it != ui_overlays_.end(); ++it )
  {
    if ( *it != overlay )
    {
      continue;
    }

    if ( renderer_ != nullptr )
    {
      renderer_->removeOverlay( overlay );
    }

    disconnect( static_cast<UiOverlay *>(overlay.get()), &UiOverlay::zIndexChanged, this,
                &OverlayManager::onZIndexChanged );
    ui_overlays_.erase( it );

    checkEmpty();
    return true;
  }

  for ( auto it = popup_overlays_.begin(); it != popup_overlays_.end(); ++it )
  {
    if ( *it != overlay )
    {
      continue;
    }

    if ( renderer_ != nullptr )
    {
      renderer_->removeOverlay( overlay );
    }
    popup_overlays_.erase( it );

    checkEmpty();
    return true;
  }
  return false;
}

OverlayPtr OverlayManager::getByName( const std::string &name )
{
  std::unique_lock<std::recursive_mutex> scoped_lock( mutex_ );
  for ( UiOverlayPtr &overlay : ui_overlays_ )
  {
    if ( overlay->name() == name ) return overlay;
  }
  for ( PopupOverlayPtr &overlay : popup_overlays_ )
  {
    if ( overlay->name() == name ) return overlay;
  }
  return nullptr;
}

OverlayConstPtr OverlayManager::getByName( const std::string &name ) const
{
  std::unique_lock<std::recursive_mutex> scoped_lock( mutex_ );
  for ( const UiOverlayPtr &overlay : ui_overlays_ )
  {
    if ( overlay->name() == name ) return overlay;
  }
  for ( const PopupOverlayPtr &overlay : popup_overlays_ )
  {
    if ( overlay->name() == name ) return overlay;
  }
  return nullptr;
}

void OverlayManager::onZIndexChanged()
{
  auto overlay = static_cast<UiOverlay *>(QObject::sender());
  auto element = std::find_if( ui_overlays_.begin(), ui_overlays_.end(),
                               [ = ]( const OverlayPtr &x )
                               {
                                 return x.get() == overlay;
                               } );
  if ( element == ui_overlays_.end()) return;
  UiOverlayPtr ui_overlay_ptr = *element;

  // Remove overlay and insert it back so that the overlays vector stays ordered from lowest z index to highest
  ui_overlays_.erase( element );
  OverlayPtr overlay_ptr = std::static_pointer_cast<Overlay>( ui_overlay_ptr );
  renderer_->removeOverlay( overlay_ptr );
  insertOverlay( ui_overlay_ptr );
}

void OverlayManager::moveToFront( UiOverlayPtr overlay )
{
  UiOverlayPtr top = ui_overlays_[ui_overlays_.size() - 1];
  if ( top == overlay ) return;

  overlay->setZIndex( top->zIndex() + 1 );
}

void OverlayManager::moveToFront( const std::string &name )
{
  OverlayPtr overlay = getByName( name );
  UiOverlayPtr ui_overlay = std::dynamic_pointer_cast<UiOverlay>( overlay );
  if ( ui_overlay == nullptr )
  {
    throw std::invalid_argument( "An UI overlay with the given name was not found!" );
  }
  moveToFront( ui_overlay );
}

void OverlayManager::moveToBack( const std::string &name )
{
  OverlayPtr overlay = getByName( name );
  UiOverlayPtr ui_overlay = std::dynamic_pointer_cast<UiOverlay>( overlay );
  if ( ui_overlay == nullptr )
  {
    throw std::invalid_argument( "An overlay with the given name was not found!" );
  }
  moveToBack( ui_overlay );
}

void OverlayManager::moveToBack( UiOverlayPtr overlay )
{
  UiOverlayPtr bottom = ui_overlays_[0];
  if ( bottom == overlay ) return;

  overlay->setZIndex( bottom->zIndex() - 1 );
}

void OverlayManager::insertOverlay( UiOverlayPtr overlay )
{
  OverlayPtr overlay_ptr = std::static_pointer_cast<Overlay>( overlay );
  std::unique_lock<std::recursive_mutex> scoped_lock( mutex_ );
  int i = 0;
  for ( auto it = ui_overlays_.begin(); it != ui_overlays_.end(); ++it, ++i )
  {
    if ((*it)->zIndex() <= overlay->zIndex()) continue;

    ui_overlays_.insert( it, overlay );
    renderer_->insertOverlay( overlay_ptr, i );
    return;
  }
  ui_overlays_.push_back( overlay );
  if ( popup_overlays_.empty())
  {
    renderer_->addOverlay( overlay_ptr );
  }
  else
  {
    renderer_->insertOverlay( overlay_ptr, i );
  }
}

bool OverlayManager::eventFilter( QObject *receiver, QEvent *event )
{
  if ( receiver == nullptr ) return false;
  if ( sending_event_ ) return false;
  if ( !renderer_->isRendering()) return false;
  if ( !render_panel_->isActiveWindow()) return false; // TODO actually we just need to know if there is a dialog in front

  sending_event_ = true;
  switch ( event->type())
  {
    case QEvent::MouseMove:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::MouseButtonDblClick:
      if ( handleMouseEvent( receiver, static_cast<QMouseEvent *>(event)))
      {
        sending_event_ = false;
        return true;
      }
      break;
    case QEvent::Wheel:
      if ( handleWheelEvent( receiver, static_cast<QWheelEvent *>(event)))
      {
        sending_event_ = false;
        return true;
      }
      break;
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
      if ( handleKeyEvent( receiver, static_cast<QKeyEvent *>(event)))
      {
        sending_event_ = false;
        return true;
      }
      break;
    default:
      break;
  }
  sending_event_ = false;
  return QObject::eventFilter( receiver, event );
}

bool OverlayManager::handleMouseEvent( QObject *receiver, QMouseEvent *event )
{
  bool is_mouse_down = (event->buttons() & Qt::LeftButton) == Qt::LeftButton;
  QPoint render_panel_pos = render_panel_->mapFromGlobal( event->globalPos());
  if ( !render_panel_->contentsRect().contains( render_panel_pos ) && mouse_down_overlay_ == nullptr )
  {
    if ( is_mouse_down ) focused_overlay_ = nullptr;
    if ( mouse_overlay_ != nullptr )
    {
      mouse_overlay_->handleEventsCanceled();
      mouse_overlay_ = nullptr;
    }
    return false;
  }

  // If the mouse is down, not just pressed and the pressed wasn't handled by the overlay, ignore subsequent events
  // until mouse is up again.
  if ( is_mouse_down && (event->type() != QEvent::MouseButtonPress || event->button() != Qt::LeftButton) &&
       mouse_down_overlay_ == nullptr )
  {
    return false;
  }
  // Again, if the mouse down was not handled by the overlay, the mouse up should also not be handled by the overlay to
  // prevent ending up in a state where rviz thinks the mouse down is theirs when the overlay handled and consumed it.
  if ( event->type() == QEvent::MouseButtonRelease && event->button() == Qt::LeftButton &&
       mouse_down_overlay_ == nullptr )
  {
    return false;
  }

  QMouseEvent child_mouse_event( event->type(), render_panel_pos, event->screenPos(),
                                 event->button(), event->buttons(), event->modifiers());
  if ( mouse_down_overlay_ != nullptr )
  {
    bool result = mouse_down_overlay_->handleEvent( receiver, &child_mouse_event );
    if ( !is_mouse_down )
    {
      mouse_down_overlay_ = nullptr;
    }
    event->setAccepted( child_mouse_event.isAccepted());
    return result;
  }

  std::unique_lock<std::recursive_mutex> scoped_lock( mutex_ );
  // First go through popups because they are on top
  for ( int i = (int) popup_overlays_.size() - 1; i >= 0; --i )
  {
    if ( !popup_overlays_[i]->isVisible()) continue;

    if ( !popup_overlays_[i]->handleEvent( receiver, &child_mouse_event )) continue;

    PopupOverlayPtr popup_overlay = popup_overlays_[i];
    if ( mouse_overlay_ != popup_overlay && mouse_overlay_ != nullptr )
    {
      mouse_overlay_->handleEventsCanceled();
    }

    if ( i == popup_overlays_.size() - 1 )
    {
      // The popup that is at the top did not handle the event but another one did, hence, we move the other to the top.
      popup_overlays_.erase( popup_overlays_.begin() + i );
      popup_overlays_.push_back( popup_overlay );
      OverlayPtr overlay = std::static_pointer_cast<Overlay>( popup_overlay );
      renderer_->removeOverlay( overlay );
      renderer_->addOverlay( overlay );
    }

    mouse_overlay_ = popup_overlay;
    if ( is_mouse_down )
    {
      mouse_down_overlay_ = popup_overlay;
      focused_overlay_ = popup_overlay;
    }
    event->setAccepted( true );
    return true;
  }

  // Go through overlays from last to first, because the last one is drawn on top of the others
  for ( int i = (int) ui_overlays_.size() - 1; i >= 0; --i )
  {
    if ( !ui_overlays_[i]->isVisible()) continue;

    if ( !ui_overlays_[i]->handleEvent( receiver, &child_mouse_event )) continue;
    if ( mouse_overlay_ != ui_overlays_[i] && mouse_overlay_ != nullptr )
    {
      mouse_overlay_->handleEventsCanceled();
    }

    mouse_overlay_ = ui_overlays_[i];
    if ( is_mouse_down )
    {
      mouse_down_overlay_ = ui_overlays_[i];
      focused_overlay_ = ui_overlays_[i];
    }
    event->setAccepted( true );
    return true;
  }

  // If mouse is down and no overlay handled it, we reset the focused overlay so that it doesn't keep on eating all
  // scroll and key events.
  if ( is_mouse_down )
  {
    focused_overlay_ = nullptr;
  }
  return false;
}

bool OverlayManager::handleWheelEvent( QObject *receiver, QWheelEvent *event )
{
  // Map the wheel event to the render panel because the top left of the render panel is equivalent to the top left of
  // the overlays.
  QPoint render_panel_pos = render_panel_->mapFromGlobal( event->globalPos());
  if ( !render_panel_->contentsRect().contains( render_panel_pos )) return false;

  QWheelEvent child_event( render_panel_pos, event->globalPos(), event->pixelDelta(), event->angleDelta(),
                           event->delta(),
                           event->orientation(), event->buttons(), event->modifiers(), event->phase(), event->source());

  std::unique_lock<std::recursive_mutex> scoped_lock( mutex_ );
  for ( int i = (int) popup_overlays_.size() - 1; i >= 0; --i )
  {
    if ( !popup_overlays_[i]->isVisible()) continue;

    if ( !popup_overlays_[i]->handleEvent( receiver, &child_event )) continue;

    event->setAccepted( true );
    return true;
  }
  for ( int i = (int) ui_overlays_.size() - 1; i >= 0; --i )
  {
    if ( !ui_overlays_[i]->isVisible()) continue;

    if ( !ui_overlays_[i]->handleEvent( receiver, &child_event )) continue;

    event->setAccepted( true );
    return true;
  }
  return false;
}

bool OverlayManager::handleKeyEvent( QObject *receiver, QKeyEvent *event )
{
  if ( focused_overlay_ == nullptr ) return false;
  return focused_overlay_->handleEvent( receiver, event );
}
}
