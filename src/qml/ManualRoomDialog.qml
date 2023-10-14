// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

Kirigami.Dialog {
    id: root

    /**
     * @brief The connection for the current user.
     */
    required property NeoChatConnection connection

    /**
     * @brief Signal emitted when a valid room id or alias is entered.
     */
    signal roomSelected(string roomId,
                        string displayName,
                        url avatarUrl,
                        string alias,
                        string topic,
                        int memberCount,
                        bool isJoined)

    title: i18nc("@title", "Room ID or Alias")

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    standardButtons: Kirigami.Dialog.Cancel
    customFooterActions: [
        Kirigami.Action {
            enabled: roomIdAliasText.isValidText
            text: i18n("OK")
            icon.name: "dialog-ok"
            onTriggered: {
                // We don't necessarily have all the info so fill out the best we can.
                let roomId = roomIdAliasText.isAlias() ? "" : roomIdAliasText.text;
                let displayName = "";
                let avatarUrl = "";
                let alias = roomIdAliasText.isAlias() ? roomIdAliasText.text : "";
                let topic = "";
                let memberCount = -1;
                let isJoined = false;
                if (roomIdAliasText.room) {
                    roomId = roomIdAliasText.room.id;
                    displayName = roomIdAliasText.room.displayName;
                    avatarUrl = roomIdAliasText.room.avatarUrl.toString().length > 0 ? connection.makeMediaUrl(roomIdAliasText.room.avatarUrl) : ""
                    alias = roomIdAliasText.room.canonicalAlias;
                    topic = roomIdAliasText.room.topic;
                    memberCount = roomIdAliasText.room.joinedCount;
                    isJoined = true;
                }
                root.roomSelected(roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined);
                root.close();
            }
        }
    ]

    contentItem: ColumnLayout {
        spacing: 0
        FormCard.FormTextFieldDelegate {
            id: roomIdAliasText
            property bool isValidText: text.match(/(#|!)(.+):(.+)/g)
            property bool correctStart: text.startsWith("#") || text.startsWith("!")
            property NeoChatRoom room: {
                if (!acceptableInput) {
                    return null;
                }
                if (isAlias()) {
                    return root.connection.roomByAlias(text);
                } else {
                    return root.connection.room(text);
                }
            }

            label: i18n("Room ID or Alias:")
            statusMessage: {
                if (text.length > 0 && !correctStart) {
                    return i18n("Must start with # for an alias or ! for an ID");
                }
                if (timer.running) {
                    return "";
                }
                if (text.length > 0 && !isValidText) {
                    return i18n("The input is not a valid room ID or alias");
                }
                return correctStart ? "" : i18n("Must start with # for an alias or ! for an ID");
            }
            status: text.length > 0 ? Kirigami.MessageType.Error : Kirigami.MessageType.Information

            onTextEdited: timer.restart()

            function isAlias() {
                return roomIdAliasText.text.startsWith("#");
            }

            Timer {
                id: timer
                interval: 1000
            }
        }
    }

    onVisibleChanged: {
        roomIdAliasText.forceActiveFocus()
        timer.restart()
    }
}
