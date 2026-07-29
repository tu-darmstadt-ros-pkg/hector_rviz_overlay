# Hector RViz Overlay
This library allows to easily create display or other plugins for RViz that can overlay
 Qt widgets or QML items on top of the RViz RenderPanel.  
 You may also create your own hector_rviz_overlay::UiOverlay or hector_rviz_overlay::PopupOverlay subclasses to create custom overlays.

## Scientific Works
If you are using this module in a scientific context, feel free to cite this paper:
```
@INPROCEEDINGS{fabian2021hri,
  author = {Stefan Fabian and Oskar von Stryk},
  title = {Open-Source Tools for Efficient ROS and ROS2-based 2D Human-Robot Interface Development},
  year = {2021},
  booktitle = {2021 European Conference on Mobile Robots (ECMR)},
}
```

## How To Use
### QWidget
In most cases it is sufficient to simply inherit from hector_rviz_overlay::QWidgetOverlayDisplay
 and implement the onSetupUi( QWidget * ) method.  
It is *important* that the Q_OBJECT macro is used in the display declaration because the
 generated meta object is used to check for multiple instances of the overlay.  
If you want to allow multiple instances of your overlay, overwrite the default constructor
 and call the overload of the OverlayDisplay constructor that takes a bool allow_multiple
 with the value true.
  
### QML
To overlay QML, inherit from the  hector_rviz_overlay::QmlOverlayDisplay and override the
`getPathToQml()` method. This method allows to load a QML file from an absolute or a package relative
path `package://package_name/path/in/pkg.qml` (see hector_rviz_overlay::QmlOverlay::load).
Also don't forget the Q_OBJECT macro here.

### QML panel
To show QML in a dockable panel instead of on top of the 3D scene, inherit from
hector_rviz_overlay::QmlPanelDisplay and override the `getPathToQml()` method. The path may be
absolute or package relative (`package://package_name/path/in/pkg.qml`). Don't forget the Q_OBJECT
macro.

The display hosts the QML in a `QQuickView` embedded with `QWidget::createWindowContainer`, which is
registered as a dock panel using `rviz_common::Display::setAssociatedWidget` - the same mechanism
rviz's Camera display uses for its render panel. The panel is therefore a native window: it is always
stacked on top of overlapping widgets that are not native windows themselves and it does not blend
with what is behind it.
Enabling and disabling the display shows and hides the dock.
The `hector_rviz_overlay_controls/QmlPanel` display provides this with a `Path` property, so no
subclass is needed to show a QML file.

The QML API is the same as for overlays with two differences: `rviz.requestKeyFocus()` and
`rviz.releaseKeyFocus()` do nothing because the panel receives key events through normal Qt focus
handling, and position trackers created with `rviz.createPositionTracker(x, y, z)` return
coordinates in logical pixels of the 3D render panel instead of overlay coordinates.

#### rviz context property
Qml files loaded by the `QmlOverlay` or `QmlPanelDisplay` will have a rviz context property
available.
See docs (TODO).

## What does it support?
Currently, the overlay supports Mouse and Keyboard input.  
The base implementation of hector_rviz_overlay::OverlayDisplay includes properties to set the
 z-index and scale of the overlay.  
The hector_rviz_overlay::QWidgetOverlayDisplay also features a style sheet property which allows to apply a
 stylesheet to the overlays top-level QWidget.  
If you encounter any problems, feel free to hit me (up).

## Known Bugs

* The widgets - especially obvious with QTextEdit - don't look like they have focus.  
  > That's because they don't, it seems that caused by the way they are rendered, they can't have
  > focus but the widgets should act like they did except you can't switch focus using tab.
* Scroll events are consumed by transparent widgets. (This does not apply to QML)  
  > Unfortunately, there seems to be no good method to determine whether a widget should eat a scroll
  > event or not.  
  > To prevent a widget from consuming scroll events add a dynamic property
  > "IgnoreWheelEvents" and set it to the bool value true.``
* Key events crash QtWidget overlays if rendering with OpenGL. (QML overlays should work)
* Tabbing a QML panel's dock behind another dock, or closing it, unchecks the display.
  > rviz connects the dock's visibility to the display's enabled state. The QML is not unloaded but
  > live reload stops polling while the display is disabled.
* Changing the `Path` of a QML panel or overlay keeps the properties the previous QML file
  registered.
  > Registered properties are matched by name and are kept across a reload so a reload of the same
  > file does not lose user edits. Properties the new file does not register stay in the tree and a
  > property of the same name keeps the old value and type. Delete and re-add the display to get a
  > clean property tree.
