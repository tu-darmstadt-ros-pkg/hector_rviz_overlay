cmake_minimum_required(VERSION 3.10)
project(hector_rviz_overlay_demo VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif ()

find_package(ament_cmake REQUIRED)
find_package(pluginlib REQUIRED)
find_package(hector_rviz_overlay REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Quick Widgets)
set(CMAKE_AUTOMOC ON)

qt5_wrap_ui(QT_UIS media/demo_widget.ui)

set(SOURCES
  include/hector_rviz_overlay_demo/popups/simple_popups_overlay_display.hpp
  include/hector_rviz_overlay_demo/demo_overlay_display.hpp
  include/hector_rviz_overlay_demo/demo_qml_overlay_display.hpp
  include/hector_rviz_overlay_demo/demo_qml_ros_overlay_display.hpp
  src/demo_overlay_display.cpp
  src/demo_qml_overlay_display.cpp
  src/demo_qml_ros_overlay_display.cpp
  src/popups/simple_popups_overlay_display.cpp
  ${QT_UIS}
)

add_library(hector_rviz_overlay_demo SHARED ${SOURCES})
target_include_directories(hector_rviz_overlay_demo PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(hector_rviz_overlay_demo hector_rviz_overlay::hector_rviz_overlay Qt5::Core Qt5::Widgets)

install(
  TARGETS hector_rviz_overlay_demo
  EXPORT hector_rviz_overlay_demo
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  DIRECTORY media
  DESTINATION share/hector_rviz_overlay_demo
)

ament_export_targets(hector_rviz_overlay_demo)
pluginlib_export_plugin_description_file(rviz_common rviz_common_plugins.xml)
ament_package()
