import QtQuick 2.9
import QtQuick.Controls 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls.Material 2.2

AvatarContainer {
    readonly property bool isNotice: eventType === "notice"

    id: messageRow

    TextDelegate {
        maximumWidth: messageListView.width - (!sentByMe ? 40 + messageRow.spacing : 0)
        flat: isNotice
        highlighted: !sentByMe
        timeLabelVisible: Math.abs(time - aboveTime) > 600000 || index == 0
        authorLabelVisible: messageRow.avatarVisible

        displayText: display
    }
}
