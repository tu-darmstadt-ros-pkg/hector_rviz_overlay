import QtQuick 2.3
import QtQuick.Layouts 1.10
import Ros 1.0

Item {
  id: page
  width: 400
  height: 200
  anchors.fill: parent
  focus: true

  Subscriber {
    id: subscriber
    topic: "/titleMessage"
    onNewMessage: helloText.text += message.data + "\n"
  }

  Text {
    id: helloText
    text: "Publish a message to /titleMessage\n to have it appear here:\n"
    anchors.horizontalCenter: page.horizontalCenter
    font.pointSize: 24; font.bold: true
    horizontalAlignment: Text.AlignHCenter
  }


  // Transforms can be looked up or watched using a connection
  Connections {
    target: TfTransformListener
    onTransformChanged: {
      var message = TfTransformListener.lookUpTransform("base_link", "world");
      var translation = message.transform.translation;
      var orientation = message.transform.rotation;
      var text = "- Can Transform: " + TfTransformListener.canTransform("base_link", "world") + "\n" +
        "- Position: " + translation.x + ", " + translation.y + ", " + translation.z + "\n" +
        "- Orientation: " + orientation.w + ", " + orientation.x + ", " + orientation.y + ", "+ orientation.z + "\n" +
        "- Valid: " + message.valid;
      // The valid property is added to each transform message to indicate if the content is valid
      // Reasons for it to be invalid include: No message received yet or one of the possible exceptions when looking up
      // the transform. If an exception occured, the message will have an 'exception' field with the type and a 'message'
      // field with the exception message.
      transformConnectionText.text = text;
   }
  }

  // Or using a TfTransform object
  TfTransform {
    id: tfTransform
    sourceFrame: "base_link"
    targetFrame: "world"
  }

  Rectangle {
    color: "#cc888888"
    anchors.bottom: page.bottom
    anchors.right: page.right
    implicitHeight: tfLayout.implicitHeight
    implicitWidth: Math.min(tfLayout.implicitWidth, parent.width / 2)
    ColumnLayout {
      id: tfLayout
      width: parent.width

      Text {
        Layout.fillWidth: true
        text: "Demonstration of a tf lookup using the TfTransformListener singleton.\nDisplays the transform from source frame 'world' to target frame 'base_link'."
        wrapMode: Text.WordWrap
      }

      Text {
        Layout.fillWidth: true
        text: "- Can Transform: Unknown\n- Position: Unknown\n- Orientation: Unknown\n- Valid: Unknown"
        wrapMode: Text.WordWrap
      }

      Text {
        Layout.fillWidth: true
        text: "\nDemo of the TfTransform component.\nDisplays the opposite transform."
        wrapMode: Text.WordWrap
      }

      Text {
        Layout.fillWidth: true
        // The translation and rotation can either be accessed using the message field as in the lookUpTransform case or,
        // alternatively, using the convenience properties translation and rotation which resolve to the message fields.
        // In either case, the message.valid field should NOT be ignored.
        text: "- Position: " + tfTransform.message.transform.translation.x + ", " + tfTransform.translation.y + ", " + tfTransform.translation.z + "\n" +
              "- Orientation: " + tfTransform.message.transform.rotation.w + ", " + tfTransform.rotation.x + ", " + tfTransform.rotation.y + ", " + tfTransform.rotation.z + "\n" +
              "- Valid: " + tfTransform.message.valid + "\n" +
              "- Exception: " + tfTransform.message.exception + "\n" +
              "- Message: " + tfTransform.message.message
        wrapMode: Text.WordWrap
      }
    }
  }
}