// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as Components

import org.kde.neochat

/**
 * @brief Component for finding rooms for the public list.
 *
 * This component is based on a SearchPage, adding the functionality to select or
 * enter a server in the header, as well as the ability to manually type a room in
 * if the public room search cannot find it.
 *
 * @sa SearchPage
 */
SearchPage {
    id: root

    /**
     * @brief The connection for the current local user.
     */
    required property NeoChatConnection connection

    title: i18nc("@action:title", "Find Your Friends")

    Component.onCompleted: focusSearch()

    model: UserDirectoryListModel {
        id: userSearchModel
        connection: root.connection
    }

    modelDelegate: Delegates.RoundedItemDelegate {
        id: userDelegate
        required property string userId
        required property string displayName
        required property url avatarUrl
        required property var directChatExists

        text: displayName

        onClicked: {
            root.connection.openOrCreateDirectChat(userDelegate.userId)
            root.closeDialog()
        }

        contentItem: RowLayout {
            spacing: Kirigami.Units.smallSpacing

            Components.Avatar {
                Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                Layout.alignment: Qt.AlignTop

                source: userDelegate.avatarUrl
                name: userDelegate.displayName
            }
            Delegates.SubtitleContentItem {
                itemDelegate: userDelegate
                subtitle: userDelegate.userId
                labelItem.textFormat: Text.PlainText
            }
            QQC2.Label {
                visible: userDelegate.directChatExists
                text: i18n("Friends")
                textFormat: Text.PlainText
                color: Kirigami.Theme.positiveTextColor
            }
        }
    }

    searchFieldPlaceholder: i18n("Find your friends…")
    noSearchPlaceholderMessage: i18n("Enter text to start searching for your friends")
    noResultPlaceholderMessage: i18nc("@info:label", "No matches found")

    Component {
        id: manualRoomDialog
        ManualRoomDialog {}
    }

    QtObject {
        id: _private
        function openManualRoomDialog() {
            let dialog = manualRoomDialog.createObject(applicationWindow().overlay, {connection: root.connection});
            dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                root.roomSelected(roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined);
                root.closeDialog();
            });
            dialog.open();
        }
    }
}
