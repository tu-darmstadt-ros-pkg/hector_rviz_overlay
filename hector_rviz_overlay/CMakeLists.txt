cmake_minimum_required(VERSION 3.1)
project(hector_rviz_overlay VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 11)

# Comment to disable rendering with OpenGL. This will also disable support for QML
add_definitions(-DRENDER_OVERLAYS_USING_OPENGL)
# Uncomment to print the render time in ms in each frame (Useful for debugging)
#add_definitions(-DDRAW_RENDERTIME)
# Uncomment to check for OpenGL errors on each call
#add_definitions(-DDEBUG_GL_CALLS)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rviz
)

find_package(Qt5 COMPONENTS Core Widgets Gui Quick Qml REQUIRED)
find_package(GLEW REQUIRED)

# We also use Ogre
include($ENV{ROS_ROOT}/core/rosbuild/FindPkgConfig.cmake)
pkg_check_modules(OGRE OGRE)
link_directories( ${OGRE_LIBRARY_DIRECTORIES} )

set(CMAKE_AUTOMOC ON)

include_directories(
  include
  ${OGRE_INCLUDE_DIRS}
  ${catkin_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
  ${Qt5Gui_INCLUDE_DIRS}
  ${Qt5Quick_INCLUDE_DIRS}
  ${Qt5Qml_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
)

set(SOURCES
  src/displays/overlay_display.cpp
  src/displays/qml_overlay_display.cpp
  src/displays/qwidget_overlay_display.cpp
  src/events/qwidget_event_manager.cpp
  src/helper/file_system_watcher.cpp
  src/helper/qml_rviz_context.cpp
  src/helper/qml_rviz_property.cpp
  src/helper/qml_tool_manager.cpp
  src/helper/rviz_tool_icon_provider.cpp
  src/popup/positioning/center_tracker.cpp
  src/popup/positioning/ogre_tracker.cpp
  src/popup/positioning/popup_move_bar.cpp
  src/popup/popup_container_widget.cpp
  src/popup/popup_overlay.cpp
  src/popup/popup_dialog.cpp
  src/popup/qwidget_popup_factory.cpp
  src/popup/qwidget_popup_overlay.cpp
  src/positioning/ogre_position_tracker.cpp
  src/positioning/position_tracker.cpp
  src/render/glx_overlay_renderer.cpp
  src/render/image_overlay_renderer.cpp
  src/render/overlay_renderer.cpp
  src/render/qopengl_wrapper.cpp
  src/render/texture_overlay_renderer.cpp
  src/ui/qml_overlay.cpp
  src/ui/qwidget_overlay.cpp
  src/ui/ui_overlay.cpp
  src/overlay.cpp
  src/overlay_manager.cpp
  src/overlay_widget.cpp
  src/path_helper.cpp
)

set(HEADERS
  include/hector_rviz_overlay/displays/overlay_display.h
  include/hector_rviz_overlay/displays/qml_overlay_display.h
  include/hector_rviz_overlay/displays/qwidget_overlay_display.h
  include/hector_rviz_overlay/events/qwidget_event_manager.h
  include/hector_rviz_overlay/helper/file_system_watcher.h
  include/hector_rviz_overlay/helper/qml_rviz_context.h
  include/hector_rviz_overlay/helper/qml_rviz_property.h
  include/hector_rviz_overlay/helper/qml_tool_manager.h
  include/hector_rviz_overlay/helper/rviz_tool_icon_provider.h
  include/hector_rviz_overlay/popup/positioning/anchor_point.h
  include/hector_rviz_overlay/popup/positioning/center_tracker.h
  include/hector_rviz_overlay/popup/positioning/ogre_tracker.h
  include/hector_rviz_overlay/popup/positioning/point_tracker.h
  include/hector_rviz_overlay/popup/positioning/popup_move_bar.h
  include/hector_rviz_overlay/popup/popup_container_widget.h
  include/hector_rviz_overlay/popup/popup_overlay.h
  include/hector_rviz_overlay/popup/popup_dialog.h
  include/hector_rviz_overlay/popup/qwidget_popup_factory.h
  include/hector_rviz_overlay/popup/qwidget_popup_overlay.h
  include/hector_rviz_overlay/positioning/ogre_position_tracker.h
  include/hector_rviz_overlay/positioning/position_tracker.h
  include/hector_rviz_overlay/render/glx_overlay_renderer.h
  include/hector_rviz_overlay/render/image_overlay_renderer.h
  include/hector_rviz_overlay/render/overlay_renderer.h
  include/hector_rviz_overlay/render/qopengl_wrapper.h
  include/hector_rviz_overlay/render/renderer.h
  include/hector_rviz_overlay/render/texture_overlay_renderer.h
  include/hector_rviz_overlay/ui/qml_overlay.h
  include/hector_rviz_overlay/ui/qwidget_overlay.h
  include/hector_rviz_overlay/ui/ui_overlay.h
  include/hector_rviz_overlay/overlay.h
  include/hector_rviz_overlay/overlay_manager.h
  include/hector_rviz_overlay/overlay_widget.h
  include/hector_rviz_overlay/path_helper.h
  src/render/gl_helpers.h
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS roscpp rviz
  DEPENDS Qt5Core Qt5Widgets Qt5Gui Qt5Widgets Qt5Quick Qt5Qml GLEW
)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Widgets Qt5::Gui Qt5::Quick Qt5::Qml ${Boost_LIBRARIES} ${GLEW_LIBRARIES} ${catkin_LIBRARIES})

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})
