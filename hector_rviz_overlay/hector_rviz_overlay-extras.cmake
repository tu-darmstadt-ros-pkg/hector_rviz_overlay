# find package the Qt6 components otherwise Qt6 will be included without components
# which will cause a build error
find_package(Qt6 REQUIRED QUIET COMPONENTS Core Widgets Gui OpenGL Quick Qml)
