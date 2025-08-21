// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as Components

import org.kde.neochat

/**
 * @brief Component for finding users from the public list.
 *
 * This component is based on a SearchPage and allows the user to enter a search
 * term into the input field and then search the room for messages with text that
 * matches the input.
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
            root.connection.requestDirectChat(userDelegate.userId);
            root.closeDialog();
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

    noSearchHelpfulAction: noResultHelpfulAction

    noResultHelpfulAction: Kirigami.Action {
        icon.name: "list-add-user"
        text: i18nc("@action:button", "Enter a User ID")
        onTriggered: _private.openManualUserDialog()
        tooltip: text
    }

    Component {
        id: manualUserDialog
        ManualUserDialog {}
    }

    QtObject {
        id: _private
        function openManualUserDialog() {
            let dialog = manualUserDialog.createObject(this, {
                connection: root.connection
            });
            dialog.parent = root.Window.window.overlay;
            dialog.accepted.connect(() => {
                root.closeDialog();
            });
            dialog.userSelected.connect(userId => {
                root.connection.requestDirectChat(userId);
            });
            dialog.open();
        }
    }
}
