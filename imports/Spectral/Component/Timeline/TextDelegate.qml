import QtQuick 2.12

import org.kde.kirigami 2.4 as Kirigami

Text {
    text: "<style>pre {white-space: pre-wrap} a{color: " + Kirigami.Theme.linkColor + ";} .user-pill{}</style>" + display

    font {
        pixelSize: Kirigami.Theme.defaultFont.pixelSize * 1.2
        family: Kirigami.Theme.defaultFont.family
    }

    color: Kirigami.Theme.textColor
//    selectionColor: Kirigami.Theme.highlightColor
//    selectedTextColor: Kirigami.Theme.highlightedTextColor

//    selectByMouse: true
//    readOnly: true
    wrapMode: Text.WordWrap
    width: parent.width
    textFormat: Text.RichText
}

