import QtQuick 2.3
//import QtAudioEngine 1.1

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

    /*Rectangle {
        width: 20
        height: 20
        color: "red"
        x: mouseArea.mouseX - mouseArea.x - 10
        y: mouseArea.mouseY - mouseArea.y - 10
    }*/

    MouseArea { z: 1; id: mouseArea; anchors.fill: helloText; hoverEnabled: true; propagateComposedEvents: true }

    // Easter Eggs
    property var mlgCounter : 0

    Keys.onReleased: {
      helloText.text = "Hello Keys"
      helloText.text = helloText.text + event.toString()
      if (event.key == Qt.Key_M && mlgCounter == 0) {
        mlgCounter = 1;
        helloText.text = "Hello 1"
      } else if (mlgCounter == 1 && event.key == Qt.Key_L) {
        mlgCounter = 2;
        helloText.text = "Hello 2"
      } else if (mlgCounter == 2 && event.key == Qt.Key_G) {
        // TODO: Run mlg stuff
        particles.running = !particles.running
        mlgCounter = 3;
        helloText.text = "Hello 3"
      } else {
        mlgCounter = 0;
      }
    }
}