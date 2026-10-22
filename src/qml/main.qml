import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: 400
    height: 400

    property bool cursorBlink: true

    // 500ms blinking timer for the retro terminal cursor
    Timer {
        interval: 500
        running: true
        repeat: true
        onTriggered: root.cursorBlink = !root.cursorBlink
    }

    Rectangle {
        anchors.fill: parent
        color: "#050805"
    }

    Flickable {
        id: flick
        anchors.fill: parent
        anchors.margins: 40
        contentWidth: termText.width
        contentHeight: termText.height
        clip: true

        Text {
            id: termText
            width: flick.width
            // Draws text + blinking retro block cursor inline right at the end!
            text: pty.textBuffer + (root.cursorBlink ? "█" : " ")
            color: "#00ff66"
            font.family: "Monospace"
            font.pixelSize: 13
            wrapMode: Text.WrapAnywhere
            textFormat: Text.PlainText

            onTextChanged: {
                flick.contentY = Math.max(0, termText.height - flick.height + 20)
            }
        }
    }
}
