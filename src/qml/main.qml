import QtQuick
import QtQuick.Controls

Item {
    id: root
    width: 400
    height: 400
    focus: true

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
            text: pty.textBuffer
            color: "#00ff66"
            font.family: "Monospace"
            font.pixelSize: 13
            wrapMode: Text.WrapAnywhere
            textFormat: Text.PlainText

            onTextChanged: {
                // Auto-scroll to bottom as new terminal output arrives
                flick.contentY = Math.max(0, termText.height - flick.height)
            }
        }
    }

    // Capture physical Bluetooth Keyboard events
    Keys.onPressed: function(event) {
        if (event.modifiers & Qt.ControlModifier) {
            if (event.key === Qt.Key_C) { pty.sendKey("\x03"); event.accepted = true; return; }
            if (event.key === Qt.Key_D) { pty.sendKey("\x04"); event.accepted = true; return; }
            if (event.key === Qt.Key_L) { pty.sendKey("\x0c"); event.accepted = true; return; }
        }

        switch (event.key) {
            case Qt.Key_Return:
            case Qt.Key_Enter:
                pty.sendKey("\r");
                break;
            case Qt.Key_Backspace:
                pty.sendKey("\x7f");
                break;
            case Qt.Key_Tab:
                pty.sendKey("\t");
                break;
            case Qt.Key_Escape:
                pty.sendKey("\x1b");
                break;
            case Qt.Key_Up:
                pty.sendKey("\x1b[A");
                break;
            case Qt.Key_Down:
                pty.sendKey("\x1b[B");
                break;
            case Qt.Key_Right:
                pty.sendKey("\x1b[C");
                break;
            case Qt.Key_Left:
                pty.sendKey("\x1b[D");
                break;
            default:
                if (event.text.length > 0) {
                    pty.sendKey(event.text);
                }
                break;
        }
        event.accepted = true;
    }
}