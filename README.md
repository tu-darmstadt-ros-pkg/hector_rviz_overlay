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

#### rviz context property
Qml files loaded by the `QmlOverlay` will have a rviz context property available.
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
* I don't find the jokes in this README THAT funny anymore but keeping them for legacy reasons.

## Who's a good boy?
You are!
