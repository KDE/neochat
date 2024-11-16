// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

ColumnLayout {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom room

    Layout.fillWidth: true
    Layout.alignment: Qt.AlignVCenter
    spacing: 0

    Item {
        Layout.fillWidth: true
        Layout.preferredHeight: Kirigami.Units.largeSpacing * 2
    }

    QQC2.AbstractButton {
        Layout.preferredWidth: Math.round(Kirigami.Units.gridUnit * 3.5)
        Layout.preferredHeight: Math.round(Kirigami.Units.gridUnit * 3.5)
        Layout.alignment: Qt.AlignHCenter

        onClicked: {
            RoomManager.resolveResource(root.room.directChatRemoteMember.uri)
        }

        contentItem: KirigamiComponents.Avatar {
            name: root.room ? root.room.displayName : ""
            source: root.room ? root.room.avatarMediaUrl : ""

            Rectangle {
                visible: root.room.usesEncryption
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
        }
    }

    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        Kirigami.Icon {
            id: securityIcon
            //TODO figure out how to make this update
            source: room.connection.isUserVerified(root.room.directChatRemoteMember.id) ?
                        (room.connection.allSessionsSelfVerified(root.room.directChatRemoteMember.id) ? "security-high" : "security-medium")
                        : "security-low"

        }
        Kirigami.Heading {
            type: Kirigami.Heading.Type.Primary
            wrapMode: QQC2.Label.Wrap
            text: root.room.displayName
            textFormat: Text.PlainText
            horizontalAlignment: Text.AlignHCenter
        }
        Item {
            Layout.preferredWidth: visible ? securityIcon.width : 0
            visible: securityIcon.visible
        }
    }
}
