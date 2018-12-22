import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12

import Spectral.Setting 0.1

Label {
    text: "<b>" + author.displayName + "</b> " + display
    color: MPalette.foreground
    font.pixelSize: 13
    font.weight: Font.Medium

    topPadding: 8
    bottomPadding: 8
    leftPadding: 24
    rightPadding: 24

    wrapMode: Label.Wrap
    textFormat: MSettings.richText ? Text.RichText : Text.StyledText
    onLinkActivated: Qt.openUrlExternally(link)

    background: Rectangle {
        color: MPalette.banner
        radius: 4
    }
}
