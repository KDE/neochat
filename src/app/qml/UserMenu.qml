// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat
import org.kde.neochat.settings
import org.kde.neochat.devtools

KirigamiComponents.ConvergentContextMenu {
    id: root

    required property Kirigami.ApplicationWindow window
    required property NeochatRoomMember author

    headerContentItem: RowLayout {
        id: detailRow

        spacing: Kirigami.Units.largeSpacing

        KirigamiComponents.Avatar {
            id: avatar
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium

            name: root.author.displayName
            source: root.author.avatarUrl
            color: root.author.color
        }

        ColumnLayout {
            Layout.fillWidth: true

            spacing: 0

            Kirigami.Heading {
                level: 1
                Layout.fillWidth: true
                font.bold: true

                elide: Text.ElideRight
                wrapMode: Text.NoWrap
                text: root.author.displayName
                textFormat: Text.PlainText
            }

            QQC2.Label {
                id: idLabel
                textFormat: TextEdit.PlainText
                text: root.author.id
                elide: Qt.ElideRight
            }
        }
    }

    QQC2.Action {
        text: i18nc("@action:button", "Open Profile")
        icon.name: "im-user-symbolic"
        onTriggered: RoomManager.resolveResource(root.author.uri)
    }

    QQC2.Action {
        text: i18nc("@action:button", "Mention")
        icon.name: "username-copy-symbolic"
        onTriggered: {
            RoomManager.currentRoom.mainCache.mentionAdded(root.author.disambiguatedName, "https://matrix.to/#/" + root.author.id);
        }
    }
}
