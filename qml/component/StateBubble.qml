import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

Rectangle {
    readonly property bool isEmote: eventType === "emote"

    id: stateRect

    width: Math.min(stateText.implicitWidth + 24, messageListView.width)
    height: stateText.implicitHeight + 24

    color: isEmote ? Material.accent : background

    Label {
        id: stateText
        text: "<b>" + author.displayName + "</b> " + display
        color: isEmote ? "white" : Material.foreground
        linkColor: isEmote || darkTheme ? "white" : Material.accent
        anchors.fill: parent
        anchors.margins: 12
        wrapMode: Label.Wrap
        textFormat: Text.StyledText
        onLinkActivated: Qt.openUrlExternally(link)
    }
}
