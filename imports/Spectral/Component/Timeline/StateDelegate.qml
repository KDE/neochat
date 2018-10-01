import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

import Spectral.Setting 0.1

Label {
    Layout.alignment: Qt.AlignHCenter

    text: "<b>" + author.displayName + "</b> " + display
    color: "white"

    padding: 8

    wrapMode: Label.Wrap
    linkColor: "white"
    textFormat: MSettings.richText ? Text.RichText : Text.StyledText
    onLinkActivated: Qt.openUrlExternally(link)

    background: Rectangle {
        color: MSettings.darkTheme ? "#484848" : "grey"
    }
}
