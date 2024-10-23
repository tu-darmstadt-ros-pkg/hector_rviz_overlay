# find package the Qt5 components otherwise Qt5 will be included without components
# which will cause a build error
find_package(Qt5 REQUIRED QUIET COMPONENTS Core Widgets Gui Quick Qml)
