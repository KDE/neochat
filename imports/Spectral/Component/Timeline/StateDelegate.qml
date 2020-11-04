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

    Kirigami.Avatar {
        Layout.preferredWidth: 24
        Layout.preferredHeight: 24

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
        font.pixelSize: 13
        font.bold: true
    }

    Label {
        Layout.fillWidth: true

        text: display
        color: MPalette.foreground
        font.pixelSize: 13
        font.weight: Font.Medium

        wrapMode: Label.Wrap
        onLinkActivated: Qt.openUrlExternally(link)
    }
}
