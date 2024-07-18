import QtQuick 2.3

Item {
    id: page
    width: 400
    height: 200
    anchors.fill: parent
    focus: true

    Text {
        id: helloText
        text: "Hello Germany!"
        anchors.verticalCenter: page.verticalCenter
        anchors.horizontalCenter: page.horizontalCenter
        font.pointSize: 24; font.bold: true

        states: State {
            name: "down"; when: mouseArea.pressed == true
            PropertyChanges { target: helloText; y: 160; rotation: 180; color: "red"; text: "Hello Australia!" }
        }

        transitions: Transition {
            from: ""; to: "down"; reversible: true
            ParallelAnimation {
                NumberAnimation { properties: "y,rotation";  duration: 500; easing.type: Easing.InOutQuad }
                ColorAnimation { duration: 500 }
            }
        }
    }
    Text {
        anchors.bottom: page.bottom
        anchors.horizontalCenter: page.horizontalCenter
        anchors.bottomMargin: 8
        text: "Your fixed frame is: " + rviz.fixedFrame
        font.pointSize: 20; font.bold: true
    }

    MouseArea { z: 1; id: mouseArea; anchors.fill: helloText; hoverEnabled: true; propagateComposedEvents: true }
}