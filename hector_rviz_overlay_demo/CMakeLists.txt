cmake_minimum_required(VERSION 3.1)
project(hector_rviz_overlay_demo VERSION 1.0.0)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR  ON)

find_package(catkin REQUIRED COMPONENTS
  roscpp
  rviz
  hector_rviz_overlay
)

find_package(Qt5 REQUIRED Core Widgets)
set(CMAKE_AUTOMOC ON)


#include(${QT_USE_FILE})
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

qt5_wrap_ui(QT_UIS media/demo_widget.ui)

set(SOURCES
  src/demo_overlay_display.cpp
  src/demo_qml_overlay_display.cpp
  src/demo_qml_ros_overlay_display.cpp
  src/popups/simple_popups_overlay_display.cpp
  ${QT_MOC}
  ${QT_UIS}
)

set(HEADERS
  include/hector_rviz_overlay_demo/popups/simple_popups_overlay_display.h
  include/hector_rviz_overlay_demo/demo_overlay_display.h
  include/hector_rviz_overlay_demo/demo_qml_overlay_display.h
  include/hector_rviz_overlay_demo/demo_qml_ros_overlay_display.h
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS roscpp rviz
)

add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${catkin_LIBRARIES} Qt5::Core Qt5::Widgets)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})

install(DIRECTORY media DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(FILES plugin_description.xml DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
