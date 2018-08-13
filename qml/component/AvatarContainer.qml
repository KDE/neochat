import QtQuick 2.9
import QtQuick.Controls 2.2

Row {
    readonly property bool avatarVisible: !(sentByMe || (aboveAuthor === author && section === aboveSection))

    spacing: 6

    ImageStatus {
        id: avatar

        width: height
        height: 40
        round: false
        visible: avatarVisible
        source: author.avatarUrl != "" ? "image://mxc/" + author.avatarUrl : null
        displayText: author.displayName
    }

    Rectangle {
        width: height
        height: 40
        color: "transparent"
        visible: !avatarVisible
    }
}
