// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

ColumnLayout {
    Layout.fillWidth: true
    Layout.alignment: Qt.AlignVCenter
    spacing: 0

    Item {
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.largeSpacing * 2
    }
    Kirigami.Avatar {
        Layout.preferredWidth: Math.round(Kirigami.Units.gridUnit * 3.5)
        Layout.preferredHeight: Math.round(Kirigami.Units.gridUnit * 3.5)
        Layout.alignment: Qt.AlignHCenter

        name: room ? room.displayName : ""
        source: room ? ("image://mxc/" +  room.avatarMediaId) : ""

        Rectangle {
            visible: room.usesEncryption
            color: Kirigami.Theme.backgroundColor

            width: Kirigami.Units.gridUnit
            height: Kirigami.Units.gridUnit
            anchors.bottom: parent.bottom
            anchors.right: parent.right

            radius: Math.round(width / 2)

            Kirigami.Icon {
                source: "channel-secure-symbolic"
                anchors.fill: parent
            }
        }
        actions.main: Kirigami.Action {
            onTriggered: {
                const popup = userDetailDialog.createObject(QQC2.ApplicationWindow.overlay, {
                    room: room,
                    user: room.getUser(room.directChatRemoteUser.id),
                })
                popup.closed.connect(() => {
                    userListItem.highlighted = false
                })
                if (roomDrawer.modal) {
                    roomDrawer.close()
                }
                popup.open()
            }
        }
    }

    Kirigami.Heading {
        Layout.fillWidth: true
        type: Kirigami.Heading.Type.Primary
        wrapMode: QQC2.Label.Wrap
        text: room.displayName
        textFormat: Text.PlainText
        horizontalAlignment: Text.AlignHCenter
    }
}
