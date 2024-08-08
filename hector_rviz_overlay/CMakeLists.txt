cmake_minimum_required(VERSION 3.10)
project(hector_rviz_overlay VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

# Comment to disable rendering with OpenGL. This will also disable support for QML
add_definitions(-DRENDER_OVERLAYS_USING_OPENGL)
# Uncomment to print the render time in ms in each frame (Useful for debugging)
#add_definitions(-DDRAW_RENDERTIME)
# Uncomment to check for OpenGL errors on each call
#add_definitions(-DDEBUG_GL_CALLS)

find_package(ament_cmake REQUIRED)
find_package(rviz_common REQUIRED)
find_package(Qt5 COMPONENTS Core Widgets Gui Quick Qml REQUIRED)
find_package(GLEW REQUIRED)

## We also use Ogre
#include($ENV{ROS_ROOT}/core/rosbuild/FindPkgConfig.cmake)
#pkg_check_modules(OGRE OGRE)
#link_directories( ${OGRE_LIBRARY_DIRECTORIES} )

set(CMAKE_AUTOMOC ON)

set(SOURCES
  include/hector_rviz_overlay/displays/overlay_display.hpp
  include/hector_rviz_overlay/displays/qml_overlay_display.hpp
  include/hector_rviz_overlay/displays/qwidget_overlay_display.hpp
  include/hector_rviz_overlay/events/qwidget_event_manager.hpp
  include/hector_rviz_overlay/helper/file_system_watcher.hpp
  include/hector_rviz_overlay/helper/qml_rviz_context.hpp
  include/hector_rviz_overlay/helper/qml_rviz_property.hpp
  include/hector_rviz_overlay/helper/qml_tool_manager.hpp
  include/hector_rviz_overlay/helper/rviz_tool_icon_provider.hpp
  include/hector_rviz_overlay/popup/positioning/anchor_point.hpp
  include/hector_rviz_overlay/popup/positioning/center_tracker.hpp
  include/hector_rviz_overlay/popup/positioning/ogre_tracker.hpp
  include/hector_rviz_overlay/popup/positioning/point_tracker.hpp
  include/hector_rviz_overlay/popup/positioning/popup_move_bar.hpp
  include/hector_rviz_overlay/popup/popup_container_widget.hpp
  include/hector_rviz_overlay/popup/popup_overlay.hpp
  include/hector_rviz_overlay/popup/popup_dialog.hpp
  include/hector_rviz_overlay/popup/qwidget_popup_factory.hpp
  include/hector_rviz_overlay/popup/qwidget_popup_overlay.hpp
  include/hector_rviz_overlay/positioning/ogre_position_tracker.hpp
  include/hector_rviz_overlay/positioning/position_tracker.hpp
  include/hector_rviz_overlay/render/qimage_texture_overlay_renderer.hpp
  include/hector_rviz_overlay/render/overlay_renderer.hpp
  src/render/qopengl_wrapper.hpp
  include/hector_rviz_overlay/render/renderer.hpp
  include/hector_rviz_overlay/render/texture_overlay_renderer.hpp
  include/hector_rviz_overlay/ui/qml_overlay.hpp
  include/hector_rviz_overlay/ui/qwidget_overlay.hpp
  include/hector_rviz_overlay/ui/ui_overlay.hpp
  include/hector_rviz_overlay/overlay.hpp
  include/hector_rviz_overlay/overlay_manager.hpp
  include/hector_rviz_overlay/overlay_widget.hpp
  include/hector_rviz_overlay/path_helper.hpp
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
  src/render/gl_helpers.h
  src/render/qimage_texture_overlay_renderer.cpp
  src/render/overlay_renderer.cpp
  src/render/qopengl_wrapper.cpp
  src/render/texture_overlay_renderer.cpp
  src/ui/qml_overlay.cpp
  src/ui/qwidget_overlay.cpp
  src/ui/ui_overlay.cpp
  src/logging.hpp
  src/overlay.cpp
  src/overlay_manager.cpp
  src/overlay_widget.cpp
  src/path_helper.cpp
  src/render/qopengl_texture_overlay_renderer.cpp
)

# Create the library
add_library(hector_rviz_overlay SHARED ${SOURCES})
target_include_directories(hector_rviz_overlay PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# Link required libraries
target_link_libraries(hector_rviz_overlay PUBLIC
  Qt5::Core
  Qt5::Widgets
  Qt5::Gui
  Qt5::Quick
  Qt5::Qml
  rviz_common::rviz_common
)
target_link_libraries(hector_rviz_overlay PRIVATE
  ${GLEW_LIBRARIES}
)

install(TARGETS hector_rviz_overlay
  EXPORT hector_rviz_overlay
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/ DESTINATION include)

# Export package information
ament_export_include_directories(include)
ament_export_targets(hector_rviz_overlay)
ament_export_dependencies(rviz_common Qt5)
ament_package()
