// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat
import org.kde.neochat.settings

/**
 * Context menu when clicking on a room in the room list
 */
KirigamiComponents.ConvergentContextMenu {
    id: root

    property NeoChatRoom room
    required property NeoChatConnection connection
    required property Kirigami.ApplicationWindow window

    headerContentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        KirigamiComponents.Avatar {
            id: avatar
            source: room.avatarMediaUrl
            Layout.preferredWidth: Kirigami.Units.gridUnit * 2
            Layout.preferredHeight: Kirigami.Units.gridUnit * 2
        }

        Kirigami.Heading {
            level: 2
            Layout.fillWidth: true
            text: room.displayName
            wrapMode: Text.WordWrap
        }
    }

    QQC2.Action {
        text: i18nc("'Space' is a matrix space", "View Space")
        icon.name: "view-list-details"
        onTriggered: RoomManager.resolveResource(room.id)
    }

    QQC2.Action {
        text: i18nc("@action:inmenu", "Copy Space Address")
        icon.name: "edit-copy"
        onTriggered: if (room.canonicalAlias.length === 0) {
            Clipboard.saveText(room.id);
        } else {
            Clipboard.saveText(room.canonicalAlias);
        }
    }

    QQC2.Action {
        text: i18nc("'Space' is a matrix space", "Space Settings")
        icon.name: 'settings-configure-symbolic'
        onTriggered: {
            RoomSettingsView.openRoomSettings(root.room, RoomSettingsView.Space);
        }
    }

    Kirigami.Action {
        separator: true
    }

    QQC2.Action {
        text: i18nc("'Space' is a matrix space", "Leave Space…")
        icon.name: "go-previous"
        onTriggered: Qt.createComponent('org.kde.neochat', 'ConfirmLeaveDialog').createObject(root.QQC2.ApplicationWindow.window, {
            room: root.room
        }).open();
    }
}
