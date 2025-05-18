// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

SearchPage {
    id: root

    property NeoChatRoom room

    title: i18nc("@title:dialog", "Invite a User")

    searchFieldPlaceholder: i18nc("@info:placeholder", "Find a user…")
    noResultPlaceholderMessage: i18nc("@info:placeholder", "No users found")

    headerTrailing: QQC2.Button {
        icon.name: "list-add"
        display: QQC2.Button.IconOnly
        enabled: root.model.searchText.match(/@(.+):(.+)/g) && !root.room.containsUser(root.model.searchText)

        text: i18nc("@action:button", "Invite this User")

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: root.room.containsUser(root.model.searchText) ? i18nc("@info:tooltip", "User is either already a member or has been invited") : text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

        onClicked: root.room.inviteToRoom(root.model.searchText);
    }

    model: UserDirectoryListModel {
        id: userDictListModel

        connection: root.room.connection
    }

    modelDelegate: Delegates.RoundedItemDelegate {
        id: delegate

        required property string userId
        required property string displayName
        required property url avatarUrl

        text: displayName

        contentItem: RowLayout {
            KirigamiComponents.Avatar {
                Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                source: delegate.avatarUrl
                name: delegate.displayName
            }

            Delegates.SubtitleContentItem {
                itemDelegate: delegate
                subtitle: delegate.userId
                labelItem.textFormat: Text.PlainText
            }

            QQC2.ToolButton {
                id: inviteButton

                readonly property bool inRoom: root.room && root.room.containsUser(delegate.userId)

                icon.name: "document-send"
                text: i18nc("@action:button", "Send invitation")
                opacity: inRoom ? 0.5 : 1
                enabled: !inRoom

                onClicked: {
                    inviteButton.enabled = false;
                    root.room.inviteToRoom(delegate.userId);
                }

                QQC2.ToolTip.text: !inRoom ? text : i18nc("@info:tooltip", "User is either already a member or has been invited")
                QQC2.ToolTip.visible: inviteButton.hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }
}
