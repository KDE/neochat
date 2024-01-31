// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

Kirigami.ScrollablePage {
    id: root

    property NeoChatRoom room

    title: i18n("Invite a User")

    actions: [
        Kirigami.Action {
            icon.name: "dialog-close"
            text: i18nc("@action", "Cancel")
            onTriggered: root.closeDialog()
        }
    ]
    header: RowLayout {
        Layout.fillWidth: true
        Layout.margins: Kirigami.Units.largeSpacing

        Kirigami.SearchField {
            id: identifierField
            property bool isUserId: text.match(/@(.+):(.+)/g)
            Layout.fillWidth: true

            placeholderText: i18n("Find a user...")
            onAccepted: userDictListModel.search()
        }

        QQC2.Button {
            visible: identifierField.isUserId

            text: i18n("Add")
            highlighted: true

            onClicked: {
                room.inviteToRoom(identifierField.text);
            }
        }
    }

    ListView {
        id: userDictListView

        clip: true

        model: UserDirectoryListModel {
            id: userDictListModel

            connection: root.room.connection
            searchText: identifierField.text
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent

            visible: userDictListView.count < 1

            text: i18n("No users available")
        }

        delegate: Delegates.RoundedItemDelegate {
            id: delegate

            required property string userId
            required property string displayName
            required property url avatarUrl

            property bool inRoom: room && room.containsUser(userId)

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
                    icon.name: "document-send"
                    text: i18n("Send invitation")
                    checkable: true
                    checked: inRoom
                    opacity: inRoom ? 0.5 : 1

                    onToggled: {
                        if (inRoom) {
                            checked = true;
                        } else {
                            room.inviteToRoom(delegate.userId);
                            applicationWindow().pageStack.layers.pop();
                        }
                    }

                    QQC2.ToolTip.text: !inRoom ? text : i18n("User is either already a member or has been invited")
                    QQC2.ToolTip.visible: inviteButton.hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }
        }
    }
}
