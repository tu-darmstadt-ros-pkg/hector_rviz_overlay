/*
 * Copyright (C) 2018  Stefan Fabian
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

#ifndef HECTOR_RVIZ_OVERLAY_QWIDGET_POPUP_FACTORY_H
#define HECTOR_RVIZ_OVERLAY_QWIDGET_POPUP_FACTORY_H

#include "hector_rviz_overlay/popup/qwidget_popup_overlay.h"

#include "hector_rviz_overlay/popup/popup_dialog.h"

namespace hector_rviz_overlay
{

class QWidgetPopupFactory
{
public:
  QWidgetPopupOverlayPtr createMessagePopup( const std::string &name, const QString &title, const QString &message,
                                             int buttons = ButtonOk );
};
}

#endif //HECTOR_RVIZ_OVERLAY_QWIDGET_POPUP_FACTORY_H
