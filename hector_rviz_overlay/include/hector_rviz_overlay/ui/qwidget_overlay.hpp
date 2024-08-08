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

#ifndef HECTOR_RVIZ_OVERLAY_QWIDGET_OVERLAY_H
#define HECTOR_RVIZ_OVERLAY_QWIDGET_OVERLAY_H

#include "ui_overlay.hpp"

class QKeyEvent;
class QMouseEvent;
class QWheelEvent;

namespace hector_rviz_overlay
{

class OverlayWidget;
class QWidgetEventManager;

/*!
 * @class QWidgetOverlay
 * @brief This class provides a QWidget that can be used as a top level widget to draw on top of the rviz_common::RenderPanel.
 */
class QWidgetOverlay : public UiOverlay
{
Q_OBJECT
public:
  explicit QWidgetOverlay( const std::string &name );

  ~QWidgetOverlay() override;

  ///@inherit
  void prepareRender( Renderer * ) override { }

  ///@inherit
  void releaseRenderResources() override { }

  ///@inherit
  bool handleEvent( QObject *receiver, QEvent *event ) override;

  ///@inherit
  void handleEventsCanceled() override;

  ///@{
  /*!
   * @return The top level widget that is drawn on top of the rviz_common::RenderPanel.
   */
  QWidget *widget();

  const QWidget *widget() const;
  ///@}

  ///@inherit
  void setScale( float value ) override;

  ///@inherit
  void setGeometry( const QRect &value ) override;

  bool isDirty() const override;

protected:

  ///@inherit
  void renderImpl( Renderer *renderer ) override;

  OverlayWidget *widget_;
  QWidgetEventManager *event_manager_;
};

typedef std::shared_ptr<QWidgetOverlay> QWidgetOverlayPtr;
typedef std::shared_ptr<const QWidgetOverlay> QWidgetOverlayConstPtr;
}

#endif //HECTOR_RVIZ_OVERLAY_QWIDGET_OVERLAY_H
