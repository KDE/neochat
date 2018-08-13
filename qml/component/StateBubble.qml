import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Controls.Material 2.2

TextDelegate {
    maximumWidth: messageListView.width
    highlighted: eventType === "emote"
    timeLabelVisible: false
    authorLabelVisible: false

    displayText: "<b>" + author.displayName + "</b> " + display
}
