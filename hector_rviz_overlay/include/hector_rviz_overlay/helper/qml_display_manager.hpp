/*
 * Copyright (C) 2026  Stefan Fabian
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

#ifndef HECTOR_RVIZ_OVERLAY_QML_DISPLAY_MANAGER_H
#define HECTOR_RVIZ_OVERLAY_QML_DISPLAY_MANAGER_H

#include <QHash>
#include <QObject>
#include <QPointer>
#include <QVariantList>

namespace rviz_common
{
class Display;
class DisplayContext;
class DisplayGroup;
namespace properties
{
class Property;
} // namespace properties
} // namespace rviz_common

namespace hector_rviz_overlay
{

class QmlDisplayManager;

/*!
 * Wraps an rviz_common::Display for use from QML. A Display is a BoolProperty whose bool value is
 * its enabled/visibility state, so toggling visibility is just setEnabled.
 *
 * Wrappers are owned and created exclusively by QmlDisplayManager. Every accessor is guarded against
 * the wrapped display being destroyed (QPointer) and returns a sentinel value once it is gone.
 */
class QmlDisplay : public QObject
{
  Q_OBJECT
  // @formatter:off
  //! nameChanged only fires for renames made through this wrapper; a rename via the rviz properties
  //! panel goes through Display::setName (setObjectName, no changed()) and is not reflected here.
  Q_PROPERTY( QString name READ getName WRITE setName NOTIFY nameChanged )
  Q_PROPERTY( QString classId READ getClassId CONSTANT )
  Q_PROPERTY( QString description READ getDescription CONSTANT )
  Q_PROPERTY( bool enabled READ isEnabled WRITE setEnabled NOTIFY enabledChanged )
  Q_PROPERTY( bool shouldBeSaved READ shouldBeSaved WRITE setShouldBeSaved NOTIFY shouldBeSavedChanged )
  Q_PROPERTY( bool isGroup READ isGroup CONSTANT )
  Q_PROPERTY( bool valid READ valid NOTIFY invalidated )
  Q_PROPERTY( QVariantList displays READ displays NOTIFY displaysChanged )
  // @formatter:on
public:
  QString getName() const;

  void setName( const QString &name );

  QString getClassId() const;

  QString getDescription() const;

  bool isEnabled() const;

  void setEnabled( bool enabled );

  bool shouldBeSaved() const;

  void setShouldBeSaved( bool save );

  bool isGroup() const;

  bool valid() const;

  QVariantList displays() const;

  rviz_common::Display *display() const { return display_; }

  /*!
   * Resolves a property by its nested name path (e.g. "Offset/X"), splitting on '/'.
   * @return A fresh, JavaScript-owned QmlRvizProperty wrapper for the leaf, or nullptr if any
   *   segment does not exist. The returned wrapper allocates and connects on every call, so it must
   *   not be called inside a QML binding: resolve once, store it in a QML property and reuse it.
   */
  Q_INVOKABLE QObject *getProperty( const QString &path );

  Q_INVOKABLE bool setPropertyValue( const QString &path, const QVariant &value );

  Q_INVOKABLE QVariant getPropertyValue( const QString &path );

  //! Convenience wrapper for Display::setTopic used by the "New display by topic" flow.
  Q_INVOKABLE bool setTopic( const QString &topic, const QString &datatype );

  /*!
   * Finds a direct child display by name (groups only), first match wins.
   * @return The child display wrapper or nullptr if not found or this is not a group.
   */
  Q_INVOKABLE QObject *getDisplay( const QString &name );

signals:

  void nameChanged();

  void enabledChanged();

  void shouldBeSavedChanged();

  void invalidated();

  void displaysChanged();

private:
  friend class QmlDisplayManager;

  QmlDisplay( rviz_common::Display *display, QmlDisplayManager *manager );

  //! Called by the manager when the wrapped display is destroyed. Emits invalidated.
  void invalidate();

  //! Severs this wrapper's connections to the wrapped display. Called by the manager on quit so a
  //! display emitting during teardown cannot reach back into a wrapper whose QML context is gone.
  void disconnectSignals();

  //! Walks a '/'-separated name path from this display. Returns the leaf property or nullptr.
  rviz_common::properties::Property *resolveProperty( const QString &path ) const;

  QPointer<rviz_common::Display> display_;
  QmlDisplayManager *manager_;
};

/*!
 * Exposes rviz's display tree to QML. Wraps the root DisplayGroup and the DisplayContext, and is the
 * single owner of all QmlDisplay wrappers.
 */
class QmlDisplayManager : public QObject
{
  Q_OBJECT
  // @formatter:off
  Q_PROPERTY( QVariantList displays READ displays NOTIFY displaysChanged )
  // @formatter:on
public:
  explicit QmlDisplayManager( rviz_common::DisplayContext *context );

  ~QmlDisplayManager() override;

  QVariantList displays() const;

  //! Adds a display of the given class_id to the root group.
  Q_INVOKABLE QObject *addDisplay( const QString &class_id, const QString &name, bool enabled = true );

  //! Adds a display of the given class_id inside parent, a QmlDisplay wrapping a group.
  //! Returns nullptr if parent is not a valid group wrapper.
  Q_INVOKABLE QObject *addDisplay( QObject *parent, const QString &class_id, const QString &name,
                                   bool enabled = true );

  //! Convenience: adds a rviz_common/Group to the root group.
  Q_INVOKABLE QObject *addGroup( const QString &name );

  //! Convenience: adds a rviz_common/Group inside parent. Returns nullptr if parent is not a group.
  Q_INVOKABLE QObject *addGroup( QObject *parent, const QString &name );

  //! Removes and destroys the display wrapped by the given QmlDisplay.
  Q_INVOKABLE bool removeDisplay( QObject *display );

  //! Finds a top-level display by name, first match wins. Returns nullptr if not found.
  Q_INVOKABLE QObject *getDisplay( const QString &name );

  //! Returns the wrapper for the given display, creating and registering it on first request.
  QmlDisplay *wrapperFor( rviz_common::Display *display ) const;

signals:

  void displaysChanged();

private slots:

  void disconnectSignals();

  void onDisplayDestroyed( QObject *obj );

private:
  QObject *addDisplayTo( rviz_common::DisplayGroup *group, const QString &class_id,
                         const QString &name, bool enabled );

  rviz_common::DisplayContext *context_;
  rviz_common::DisplayGroup *root_;
  mutable QHash<QObject *, QmlDisplay *> wrappers_;
};
} // namespace hector_rviz_overlay

#endif // HECTOR_RVIZ_OVERLAY_QML_DISPLAY_MANAGER_H
