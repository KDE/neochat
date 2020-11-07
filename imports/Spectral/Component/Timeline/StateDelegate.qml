import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls.Material 2.12
import org.kde.kirigami 2.13 as Kirigami

import Spectral.Component 2.0
import Spectral.Dialog 2.0
import Spectral.Effect 2.0
import Spectral.Setting 0.1

RowLayout {
    id: row

    Item {
        Layout.minimumWidth: Kirigami.Units.iconSizes.medium
        Layout.preferredHeight: 1
    }

    Kirigami.Avatar {
        Layout.preferredWidth: Kirigami.Units.iconSizes.small
        Layout.preferredHeight: Kirigami.Units.iconSizes.small

        name: author.displayName
        source: author.avatarMediaId ? "image://mxc/" + author.avatarMediaId : ""
        color: author.color

        Component {
            id: userDetailDialog

            UserDetailDialog {}
        }

        RippleEffect {
            anchors.fill: parent

            circular: true

            onClicked: userDetailDialog.createObject(ApplicationWindow.overlay, {"room": currentRoom, "user": author.object, "displayName": author.displayName, "avatarMediaId": author.avatarMediaId, "avatarUrl": author.avatarUrl}).open()
        }
    }

    Label {
        Layout.alignment: Qt.AlignVCenter
        text: author.displayName
        color: Kirigami.Theme.disabledTextColor
    }

    Label {
        Layout.fillWidth: true

        text: display
        color: Kirigami.Theme.disabledTextColor
        font.weight: Font.Medium

        wrapMode: Label.Wrap
        onLinkActivated: Qt.openUrlExternally(link)
    }
}
