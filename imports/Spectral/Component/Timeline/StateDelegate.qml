import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral.Setting 0.1

Label {
    text: "<b>" + author.displayName + "</b> " + display
    color: Material.accent

    topPadding: 8
    bottomPadding: 8
    leftPadding: 16
    rightPadding: 16

    wrapMode: Label.Wrap
    linkColor: Material.accent
    textFormat: MSettings.richText ? Text.RichText : Text.StyledText
    onLinkActivated: Qt.openUrlExternally(link)

    background: Rectangle {
        color: "transparent"
        border.color: Material.accent
        border.width: 2
        radius: 18
    }
}
