import QtQuick 2.12
import QtQuick.Controls 2.12

import NeoChat.Dialog 2.0

Menu {
    signal viewSource()
    signal downloadAndOpen()
    signal saveFileAs()
    signal reply()
    signal redact()

    id: root

    MenuItem {
        text: "View Source"

        onTriggered: viewSource()
    }

    MenuItem {
        text: "Open Externally"

        onTriggered: downloadAndOpen()
    }

    MenuItem {
        text: "Save As"

        onTriggered: saveFileAs()
    }

    MenuItem {
        text: "Reply"

        onTriggered: reply()
    }

    MenuItem {
        text: "Redact"

        onTriggered: redact()
    }

    onClosed: destroy()
}
