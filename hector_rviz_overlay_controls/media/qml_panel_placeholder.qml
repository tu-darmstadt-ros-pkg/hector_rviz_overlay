import QtQuick 2.0

Rectangle {
  color: "transparent"
  // The size the panel's dock opens with, otherwise it is small enough to clip the message.
  implicitWidth: 400
  implicitHeight: 160

  SystemPalette {
    id: systemPalette
    colorGroup: SystemPalette.Active
  }

  Column {
    anchors.centerIn: parent
    width: parent.width - 32
    spacing: 8

    Text {
      width: parent.width
      horizontalAlignment: Text.AlignHCenter
      font.bold: true
      font.pixelSize: 16
      color: systemPalette.text
      text: "No QML file set"
    }

    Text {
      width: parent.width
      horizontalAlignment: Text.AlignHCenter
      wrapMode: Text.WordWrap
      color: systemPalette.text
      text: "Set the 'Path' property of this display to the QML file that should be shown here.\n" +
            "The path may be absolute or package relative using\n" +
            "package://{PACKAGE_NAME}/{PATH_IN_PACKAGE}"
    }
  }
}
