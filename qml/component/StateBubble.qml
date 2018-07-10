import QtQuick 2.11
import QtQuick.Controls 2.4
import QtQuick.Controls.Material 2.4

Rectangle {
    readonly property bool isEmote: eventType === "emote"

    id: stateRect

    width: Math.min(stateText.implicitWidth + 24, messageListView.width)
    height: stateText.implicitHeight + 24

    color: isEmote ? Material.accent : "lightgrey"

    Label {
        id: stateText
        text: "<b>" + author.displayName + "</b> " + display
        color: isEmote ? "white" : "black"
        anchors.fill: parent
        anchors.margins: 12
        wrapMode: Label.Wrap
        textFormat: Text.StyledText
    }
}
