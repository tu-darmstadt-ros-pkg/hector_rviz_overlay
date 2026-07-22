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

#include "hector_rviz_overlay/helper/qml_display_manager.hpp"
#include "hector_rviz_overlay/helper/qml_rviz_property.hpp"

#include <QApplication>
#include <QQmlEngine>
#include <QStringList>
#include <rviz_common/display.hpp>
#include <rviz_common/display_context.hpp>
#include <rviz_common/display_group.hpp>

#include "../logging.hpp"

using rviz_common::Display;
using rviz_common::DisplayGroup;
using rviz_common::properties::Property;

namespace hector_rviz_overlay
{

// ======================== QmlDisplay ========================

QmlDisplay::QmlDisplay( Display *display, QmlDisplayManager *manager )
    : display_( display ), manager_( manager )
{
  // Owned and deleted by QmlDisplayManager (wrappers_). Without CppOwnership QML would assign
  // JavaScriptOwnership to the parentless object returned via the displays property and free it
  // out from under the manager.
  QQmlEngine::setObjectOwnership( this, QQmlEngine::CppOwnership );
  // The wrapped display is a BoolProperty; its bool value is the enabled/visibility state and
  // changed() fires when it is toggled.
  connect( display, &Property::changed, this, &QmlDisplay::enabledChanged );
  if ( auto *group = dynamic_cast<DisplayGroup *>( display ) ) {
    connect( group, &DisplayGroup::displayAdded, this, &QmlDisplay::displaysChanged );
    connect( group, &DisplayGroup::displayRemoved, this, &QmlDisplay::displaysChanged );
  }
}

void QmlDisplay::invalidate() { emit invalidated(); }

void QmlDisplay::disconnectSignals()
{
  if ( display_.isNull() )
    return;
  disconnect( display_, nullptr, this, nullptr );
}

QString QmlDisplay::getName() const
{
  if ( display_.isNull() )
    return {};
  return display_->getName();
}

void QmlDisplay::setName( const QString &name )
{
  if ( display_.isNull() )
    return;
  // rviz's Display::setName uses setObjectName and emits no changed(), so we drive nameChanged here.
  display_->setName( name );
  emit nameChanged();
}

QString QmlDisplay::getClassId() const
{
  if ( display_.isNull() )
    return {};
  return display_->getClassId();
}

QString QmlDisplay::getDescription() const
{
  if ( display_.isNull() )
    return {};
  return display_->getDescription();
}

bool QmlDisplay::isEnabled() const
{
  if ( display_.isNull() )
    return false;
  return display_->isEnabled();
}

void QmlDisplay::setEnabled( bool enabled )
{
  if ( display_.isNull() )
    return;
  display_->setEnabled( enabled );
}

bool QmlDisplay::shouldBeSaved() const
{
  if ( display_.isNull() )
    return false;
  return display_->shouldBeSaved();
}

void QmlDisplay::setShouldBeSaved( bool save )
{
  if ( display_.isNull() )
    return;
  display_->setShouldBeSaved( save );
  emit shouldBeSavedChanged();
}

bool QmlDisplay::isGroup() const { return dynamic_cast<DisplayGroup *>( display_.data() ) != nullptr; }

bool QmlDisplay::valid() const { return !display_.isNull(); }

QVariantList QmlDisplay::displays() const
{
  QVariantList result;
  auto *group = dynamic_cast<DisplayGroup *>( display_.data() );
  if ( group == nullptr )
    return result;
  result.reserve( group->numDisplays() );
  for ( int i = 0; i < group->numDisplays(); ++i )
    result.append( QVariant::fromValue<QObject *>( manager_->wrapperFor( group->getDisplayAt( i ) ) ) );
  return result;
}

Property *QmlDisplay::resolveProperty( const QString &path ) const
{
  if ( display_.isNull() )
    return nullptr;
  Property *property = display_;
  const QStringList segments = path.split( '/' );
  for ( const QString &segment : segments ) {
    property = findChildProperty( property, segment );
    if ( property == nullptr )
      return nullptr;
  }
  return property;
}

QObject *QmlDisplay::getProperty( const QString &path )
{
  Property *property = resolveProperty( path );
  if ( property == nullptr )
    return nullptr;
  // Fresh, uncached wrapper: parentless, so QML takes JavaScriptOwnership. Do not call inside a
  // binding; resolve once and store the returned object in a QML property.
  return new QmlRvizProperty( property );
}

bool QmlDisplay::setPropertyValue( const QString &path, const QVariant &value )
{
  Property *property = resolveProperty( path );
  if ( property == nullptr )
    return false;
  return setPropertyValueCoerced( property, value );
}

QVariant QmlDisplay::getPropertyValue( const QString &path )
{
  Property *property = resolveProperty( path );
  if ( property == nullptr )
    return {};
  return property->getValue();
}

bool QmlDisplay::setTopic( const QString &topic, const QString &datatype )
{
  if ( display_.isNull() )
    return false;
  display_->setTopic( topic, datatype );
  return true;
}

QObject *QmlDisplay::getDisplay( const QString &name )
{
  auto *group = dynamic_cast<DisplayGroup *>( display_.data() );
  if ( group == nullptr )
    return nullptr;
  for ( int i = 0; i < group->numDisplays(); ++i ) {
    Display *child = group->getDisplayAt( i );
    if ( child->getName() == name )
      return manager_->wrapperFor( child );
  }
  return nullptr;
}

// ======================== QmlDisplayManager ========================

QmlDisplayManager::QmlDisplayManager( rviz_common::DisplayContext *context )
    : context_( context ), root_( context->getRootDisplayGroup() )
{
  connect( root_, &DisplayGroup::displayAdded, this, &QmlDisplayManager::displaysChanged );
  connect( root_, &DisplayGroup::displayRemoved, this, &QmlDisplayManager::displaysChanged );
  // Disconnect signals when the application is about to quit to avoid segfaults.
  connect( qApp, &QApplication::aboutToQuit, this, &QmlDisplayManager::disconnectSignals );
}

QmlDisplayManager::~QmlDisplayManager()
{
  // Wrappers are CppOwnership and parentless; the manager owns them. The wrapped displays belong to
  // rviz and are not destroyed here, so no destroyed() fires back into wrappers_ during teardown.
  qDeleteAll( wrappers_ );
  wrappers_.clear();
}

void QmlDisplayManager::disconnectSignals()
{
  disconnect( root_, &DisplayGroup::displayAdded, this, &QmlDisplayManager::displaysChanged );
  disconnect( root_, &DisplayGroup::displayRemoved, this, &QmlDisplayManager::displaysChanged );
  for ( auto it = wrappers_.cbegin(); it != wrappers_.cend(); ++it ) {
    disconnect( it.key(), &QObject::destroyed, this, &QmlDisplayManager::onDisplayDestroyed );
    it.value()->disconnectSignals();
  }
}

QVariantList QmlDisplayManager::displays() const
{
  QVariantList result;
  result.reserve( root_->numDisplays() );
  for ( int i = 0; i < root_->numDisplays(); ++i )
    result.append( QVariant::fromValue<QObject *>( wrapperFor( root_->getDisplayAt( i ) ) ) );
  return result;
}

QmlDisplay *QmlDisplayManager::wrapperFor( Display *display ) const
{
  auto it = wrappers_.find( display );
  if ( it != wrappers_.end() )
    return it.value();
  auto *wrapper = new QmlDisplay( display, const_cast<QmlDisplayManager *>( this ) );
  wrappers_.insert( display, wrapper );
  connect( display, &QObject::destroyed, this, &QmlDisplayManager::onDisplayDestroyed );
  return wrapper;
}

void QmlDisplayManager::onDisplayDestroyed( QObject *obj )
{
  // obj is mid-destruction and used only as the hash key. Using take (not operator[]) avoids
  // inserting on an absent key.
  QmlDisplay *wrapper = wrappers_.take( obj );
  if ( wrapper == nullptr )
    return;
  wrapper->invalidate();
  // deleteLater rather than delete: the destroyed() signal fires while a QMetaObject::activate
  // stack frame is live (e.g. during removeAllDisplays), which is exactly the crash class we avoid.
  wrapper->deleteLater();
  emit displaysChanged();
}

QObject *QmlDisplayManager::addDisplayTo( DisplayGroup *group, const QString &class_id,
                                          const QString &name, bool enabled )
{
  // Order matches VisualizationManager::addDisplay exactly: addDisplay before initialize.
  Display *d = group->createDisplay( class_id ); // FailedDisplay on a bad class_id, never nullptr
  group->addDisplay( d );
  d->initialize( context_ );
  d->setEnabled( enabled );
  d->setName( name );
  return wrapperFor( d );
}

QObject *QmlDisplayManager::addDisplay( const QString &class_id, const QString &name, bool enabled )
{
  return addDisplayTo( root_, class_id, name, enabled );
}

QObject *QmlDisplayManager::addDisplay( QObject *parent, const QString &class_id,
                                        const QString &name, bool enabled )
{
  auto *w = qobject_cast<QmlDisplay *>( parent );
  if ( w == nullptr || !w->valid() )
    return nullptr;
  auto *group = dynamic_cast<DisplayGroup *>( w->display() );
  if ( group == nullptr ) {
    LOG_WARN( "addDisplay failed: parent is not a group." );
    return nullptr;
  }
  return addDisplayTo( group, class_id, name, enabled );
}

QObject *QmlDisplayManager::addGroup( const QString &name )
{
  return addDisplay( "rviz_common/Group", name, true );
}

QObject *QmlDisplayManager::addGroup( QObject *parent, const QString &name )
{
  return addDisplay( parent, "rviz_common/Group", name, true );
}

bool QmlDisplayManager::removeDisplay( QObject *display )
{
  auto *w = qobject_cast<QmlDisplay *>( display );
  if ( w == nullptr || !w->valid() )
    return false;
  Display *d = w->display();
  auto *group = dynamic_cast<DisplayGroup *>( d->getParent() );
  if ( group == nullptr ) {
    LOG_WARN( "removeDisplay failed: display's parent is not a group." );
    return false;
  }
  group->takeDisplay( d );
  delete d; // destroyed() drives wrapper cleanup via onDisplayDestroyed
  return true;
}

QObject *QmlDisplayManager::getDisplay( const QString &name )
{
  for ( int i = 0; i < root_->numDisplays(); ++i ) {
    Display *child = root_->getDisplayAt( i );
    if ( child->getName() == name )
      return wrapperFor( child );
  }
  return nullptr;
}

} // namespace hector_rviz_overlay
