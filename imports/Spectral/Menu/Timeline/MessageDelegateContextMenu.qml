import QtQuick 2.12
import QtQuick.Controls 2.12

import Spectral.Dialog 2.0

Menu {
    readonly property string selectedText: contentLabel.selectedText

    signal viewSource()
    signal reply()
    signal redact()

    id: root

    MenuItem {
        text: "View Source"

        onTriggered: viewSource()
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
