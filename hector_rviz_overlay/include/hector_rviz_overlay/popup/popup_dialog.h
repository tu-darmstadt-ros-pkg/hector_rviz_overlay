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

#ifndef HECTOR_RVIZ_OVERLAY_POPUP_WIDGET_H
#define HECTOR_RVIZ_OVERLAY_POPUP_WIDGET_H

#include <QFrame>

class QGridLayout;
class QLabel;
class QPushButton;

namespace hector_rviz_overlay
{

enum PopupButtons
{
  ButtonOk = 0b0001,
  ButtonCancel = 0b0010,
  ButtonYes = 0b0100,
  ButtonNo = 0b1000
};

/*!
 * A popup widget that resembles a dialog and can be used to display messages etc.
 * It consists of a title with an optional PopupMoveBar, a QWidget as content and optionally buttons.
 */
class PopupDialog : public QFrame
{
Q_OBJECT
protected:
  bool event( QEvent *event ) override;

public:
  explicit PopupDialog( QWidget *parent = nullptr, Qt::WindowFlags flags = 0 );

  /*!
   * @return The title of the popup dialog.
   */
  QString title() const;

  /*!
   * @param value The text that should be set as the title of the popup dialog.
   */
  void setTitle(const QString &value);

  /*!
   * @return The widget that is the content of the popup.
   */
  QWidget *content();

  /*!
   * @return The widget that is the content of the popup.
   */
  const QWidget *content() const;

  /*!
   * Sets the content of the popup widget.
   * The passed widget's object name is changed to 'Content'.
   *
   * @param widget The widget that should be set as content of the popup.
   */
  void setContent(QWidget *widget);

  /*!
   * @return A combination of the PopupButtons flags which determine which buttons are shown.
   */
  int buttons() const;

  /*!
   * @param buttons A combination of the PopupButtons flags determining which buttons to show.
   */
  void setButtons(int buttons);

signals:
  void buttonClicked(PopupButtons button);

protected slots:
  void yesClicked();

  void noClicked();

  void okClicked();

  void cancelClicked();

protected:
  QGridLayout *layout_;
  QLabel *label_;
  QWidget *content_;
  QPushButton *button_yes_;
  QPushButton *button_no_;
  QPushButton *button_ok_;
  QPushButton *button_cancel_;
  int buttons_;
};
}

#endif //HECTOR_RVIZ_OVERLAY_POPUP_WIDGET_H
