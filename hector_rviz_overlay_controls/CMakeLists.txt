cmake_minimum_required(VERSION 3.1)
project(hector_rviz_overlay_controls VERSION 1.0.0)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_INCLUDE_CURRENT_DIR  ON)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  roscpp
  rviz
  hector_rviz_overlay
)

## System dependencies are found with CMake's conventions
#find_package(Qt5 REQUIRED Core Gui)
find_package(Qt5 REQUIRED Core Widgets)
set(CMAKE_AUTOMOC ON)

#include(${QT_USE_FILE})
include_directories(
  include
  ${catkin_INCLUDE_DIRS}
  ${Qt5Core_INCLUDE_DIRS}
  ${Qt5Widgets_INCLUDE_DIRS}
)

set(HEADERS
  include/${PROJECT_NAME}/qml_overlay_display.h
  include/${PROJECT_NAME}/tools_overlay_display.h
  include/${PROJECT_NAME}/tools_widget.h
)

set(SOURCES
  src/qml_overlay_display.cpp
  src/tools_overlay_display.cpp
  src/tools_widget.cpp
)

catkin_package(
  INCLUDE_DIRS include
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS roscpp rviz
)



add_library(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME} ${Boost_LIBRARIES} ${catkin_LIBRARIES} Qt5::Core Qt5::Widgets)

install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h")

install(TARGETS ${PROJECT_NAME} LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION})

install(DIRECTORY styles DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})

install(FILES plugin_description.xml DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION})
