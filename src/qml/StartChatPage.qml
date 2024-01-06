// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
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

    property NeoChatConnection connection

    title: i18n("Start a Chat")

    header: QQC2.Control {
        padding: Kirigami.Units.largeSpacing
        contentItem: RowLayout {
            Kirigami.SearchField {
                id: identifierField

                property bool isUserID: text.match(/@(.+):(.+)/g)

                Layout.fillWidth: true

                placeholderText: i18n("Find a user...")

                onAccepted: userDictListModel.search()
            }

            QQC2.Button {
                visible: identifierField.isUserID

                text: i18n("Chat")
                highlighted: true

                onClicked: {
                    connection.requestDirectChat(identifierField.text);
                    applicationWindow().pageStack.layers.pop();
                }
            }
        }
    }

    ListView {
        id: userDictListView

        clip: true

        spacing: Kirigami.Units.smallSpacing

        model: UserDirectoryListModel {
            id: userDictListModel

            connection: root.connection
            keyword: identifierField.text
        }

        delegate: Delegates.RoundedItemDelegate {
            id: delegate

            required property string userID
            required property string avatar
            required property string name
            required property var directChats

            text: name

            contentItem: RowLayout {
                KirigamiComponents.Avatar {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    source: delegate.avatar ? ("image://mxc/" + delegate.avatar) : ""
                    name: delegate.name
                }

                Delegates.SubtitleContentItem {
                    itemDelegate: delegate
                    subtitle: delegate.userID
                    Layout.fillWidth: true
                    labelItem.textFormat: Text.PlainText
                }

                QQC2.Button {
                    id: joinChatButton

                    visible: delegate.directChats && delegate.directChats.length > 0
                    text: i18n("Join existing chat")
                    display: QQC2.Button.IconOnly

                    icon.name: "document-send"
                    onClicked: {
                        connection.requestDirectChat(delegate.userID);
                        applicationWindow().pageStack.layers.pop();
                    }

                    Layout.alignment: Qt.AlignRight

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }

                QQC2.Button {
                    icon.name: "irc-join-channel"
                    // We wants to make sure an user can't start more than one
                    // chat with someone.
                    visible: !joinChatButton.visible
                    text: i18n("Create new chat")
                    display: QQC2.Button.IconOnly

                    onClicked: {
                        connection.requestDirectChat(delegate.userID);
                        applicationWindow().pageStack.layers.pop();
                    }

                    Layout.alignment: Qt.AlignRight

                    QQC2.ToolTip.text: text
                    QQC2.ToolTip.visible: hovered
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                }
            }
        }

        Kirigami.PlaceholderMessage {
            anchors.centerIn: parent
            visible: userDictListView.count < 1
            text: i18n("No users available")
        }
    }
}
