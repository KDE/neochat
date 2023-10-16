// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as Components

import org.kde.neochat

/**
 * @brief A dialog to select a parent space to add to a given room.
 */
Kirigami.Dialog {
    id: root

    /**
     * @brief The current room that a parent is being selected for.
     */
    required property NeoChatRoom room

    title: i18nc("@title", "Select new official parent")

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    standardButtons: Kirigami.Dialog.Cancel
    customFooterActions: [
        Kirigami.Action {
            enabled: chosenRoomDelegate.visible && root.room.canModifyParent(chosenRoomDelegate.roomId)
            text: i18n("OK")
            icon.name: "dialog-ok"
            onTriggered: {
                root.room.addParent(chosenRoomDelegate.roomId, makeCanonicalCheck.checked, existingOfficialCheck.checked)
                root.close();
            }
        }
    ]

    contentItem: ColumnLayout {
        spacing: 0
        FormCard.FormButtonDelegate {
            visible: !chosenRoomDelegate.visible
            text: i18nc("@action:button", "Pick room")
            onClicked: {
                let dialog = pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/JoinRoomPage.qml", {connection: root.room.connection, showOnlySpaces: true}, {title: i18nc("@title", "Choose Parent Space")})
                dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                    chosenRoomDelegate.roomId = roomId;
                    chosenRoomDelegate.displayName = displayName;
                    chosenRoomDelegate.avatarUrl = avatarUrl;
                    chosenRoomDelegate.alias = alias;
                    chosenRoomDelegate.topic = topic;
                    chosenRoomDelegate.memberCount = memberCount;
                    chosenRoomDelegate.isJoined = isJoined;
                    chosenRoomDelegate.visible = true;
                })
            }
        }
        FormCard.AbstractFormDelegate {
            id: chosenRoomDelegate
            property string roomId
            property string displayName
            property url avatarUrl
            property string alias
            property string topic
            property int memberCount
            property bool isJoined

            visible: false

            contentItem: RowLayout {
                Components.Avatar {
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2
                    Layout.preferredHeight: Kirigami.Units.gridUnit * 2

                    source: chosenRoomDelegate.avatarUrl
                    name: chosenRoomDelegate.displayName
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    RowLayout {
                        Layout.fillWidth: true
                        Kirigami.Heading {
                            Layout.fillWidth: true
                            level: 4
                            text: chosenRoomDelegate.displayName
                            font.bold: true
                            textFormat: Text.PlainText
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                        }
                        QQC2.Label {
                            visible: chosenRoomDelegate.isJoined
                            text: i18n("Joined")
                            color: Kirigami.Theme.linkColor
                        }
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        visible: text
                        text: chosenRoomDelegate.topic ? chosenRoomDelegate.topic.replace(/(\r\n\t|\n|\r\t)/gm," ") : ""
                        textFormat: Text.PlainText
                        elide: Text.ElideRight
                        wrapMode: Text.NoWrap
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        Kirigami.Icon {
                            source: "user"
                            color: Kirigami.Theme.disabledTextColor
                            implicitHeight: Kirigami.Units.iconSizes.small
                            implicitWidth: Kirigami.Units.iconSizes.small
                        }
                        QQC2.Label {
                            text: chosenRoomDelegate.memberCount + " " + (chosenRoomDelegate.alias ?? chosenRoomDelegate.roomId)
                            color: Kirigami.Theme.disabledTextColor
                            elide: Text.ElideRight
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            onClicked: {
                let dialog = pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/JoinRoomPage.qml", {connection: root.room.connection, showOnlySpaces: true}, {title: i18nc("@title", "Explore Rooms")})
                dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                    chosenRoomDelegate.roomId = roomId;
                    chosenRoomDelegate.displayName = displayName;
                    chosenRoomDelegate.avatarUrl = avatarUrl;
                    chosenRoomDelegate.alias = alias;
                    chosenRoomDelegate.topic = topic;
                    chosenRoomDelegate.memberCount = memberCount;
                    chosenRoomDelegate.isJoined = isJoined;
                    chosenRoomDelegate.visible = true;
                })
            }
        }
        FormCard.FormCheckDelegate {
            id: existingOfficialCheck
            property NeoChatRoom space: root.room.connection.room(chosenRoomDelegate.roomId)
            text: i18n("Set this room as a child of the space %1", space?.displayName ?? "")
            checked: enabled

            enabled: chosenRoomDelegate.visible && space && space.canSendState("m.space.child")
        }
        FormCard.FormTextDelegate {
            visible: chosenRoomDelegate.visible && !root.room.canModifyParent(chosenRoomDelegate.roomId)
            text: existingOfficialCheck.space ? (existingOfficialCheck.space.isSpace ? i18n("You do not have a high enough privilege level in the parent to set this state") : i18n("The selected room is not a space")) : i18n("You do not have the privileges to complete this action")
            textItem.color: Kirigami.Theme.negativeTextColor
        }
        FormCard.FormCheckDelegate {
            id: makeCanonicalCheck
            text: i18n("Make this space the canonical parent")
            checked: enabled

            enabled: chosenRoomDelegate.visible
        }
    }
}
