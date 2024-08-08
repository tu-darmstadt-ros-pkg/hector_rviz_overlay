cmake_minimum_required(VERSION 3.10)
project(hector_rviz_overlay_controls VERSION 1.0.0)
set (CMAKE_CXX_STANDARD 17)
set(CMAKE_INCLUDE_CURRENT_DIR  ON)

if (CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif ()


find_package(ament_cmake REQUIRED)
find_package(pluginlib REQUIRED)
find_package(hector_rviz_overlay REQUIRED)
find_package(Qt5 REQUIRED COMPONENTS Core Quick Widgets)
set(CMAKE_AUTOMOC ON)

set(SOURCES
  include/hector_rviz_overlay_controls/qml_overlay_display.h
  include/hector_rviz_overlay_controls/tools_overlay_display.h
  include/hector_rviz_overlay_controls/tools_widget.h
  src/qml_overlay_display.cpp
  src/tools_overlay_display.cpp
  src/tools_widget.cpp
)

add_library(hector_rviz_overlay_controls SHARED ${SOURCES})
target_include_directories(hector_rviz_overlay_controls PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
target_link_libraries(hector_rviz_overlay_controls hector_rviz_overlay::hector_rviz_overlay Qt5::Core Qt5::Quick Qt5::Widgets)

install(
  TARGETS hector_rviz_overlay_controls
  EXPORT hector_rviz_overlay_controls
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  DIRECTORY styles
  DESTINATION share/hector_rviz_overlay_controls
)
ament_export_targets(hector_rviz_overlay_controls)
pluginlib_export_plugin_description_file(rviz_common rviz_common_plugins.xml)
ament_package()
