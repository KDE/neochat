// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as Components

import org.kde.neochat.libneochat

Kirigami.Dialog {
    id: root

    property string parentId

    required property NeoChatConnection connection

    signal addChild(string childId, bool setChildParent, bool canonical)
    signal newChild(string childName)

    title: i18nc("@title", "Select Existing Room")
    implicitWidth: Kirigami.Units.gridUnit * 20
    standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel

    onAccepted: root.addChild(chosenRoomDelegate.roomId, existingOfficialCheck.checked, makeCanonicalCheck.checked);

    Component.onCompleted: pickRoomDelegate.forceActiveFocus()

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        FormCard.FormButtonDelegate {
            id: pickRoomDelegate

            visible: !chosenRoomDelegate.visible
            text: i18nc("@action:button", "Pick Room")
            onClicked: {
                let dialog = ((root.QQC2.ApplicationWindow.window as Kirigami.ApplicationWindow).pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat.libneochat', 'ExploreRoomsPage'), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Explore Rooms")
                });
                dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                    chosenRoomDelegate.roomId = roomId;
                    chosenRoomDelegate.displayName = displayName;
                    chosenRoomDelegate.avatarUrl = avatarUrl;
                    chosenRoomDelegate.alias = alias;
                    chosenRoomDelegate.topic = topic;
                    chosenRoomDelegate.memberCount = memberCount;
                    chosenRoomDelegate.isJoined = isJoined;
                    chosenRoomDelegate.visible = true;
                });
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
                        text: chosenRoomDelegate.topic ? chosenRoomDelegate.topic.replace(/(\r\n\t|\n|\r\t)/gm, " ") : ""
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
                let dialog = pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ExploreRoomsPage'), {
                    connection: root.connection
                }, {
                    title: i18nc("@title", "Explore Rooms")
                });
                dialog.roomSelected.connect((roomId, displayName, avatarUrl, alias, topic, memberCount, isJoined) => {
                    chosenRoomDelegate.roomId = roomId;
                    chosenRoomDelegate.displayName = displayName;
                    chosenRoomDelegate.avatarUrl = avatarUrl;
                    chosenRoomDelegate.alias = alias;
                    chosenRoomDelegate.topic = topic;
                    chosenRoomDelegate.memberCount = memberCount;
                    chosenRoomDelegate.isJoined = isJoined;
                    chosenRoomDelegate.visible = true;
                });
            }
        }

        FormCard.FormDelegateSeparator {
            below: existingOfficialCheck
        }

        FormCard.FormCheckDelegate {
            id: existingOfficialCheck
            visible: root.parentId.length > 0
            text: i18nc("@option:check As in make the space from which this dialog was created an official parent.", "Make this parent official")
            description: enabled ? i18nc("@info:description", "You have the required privilege level in the child to set this state") : i18n("You do not have a high enough privilege level in the child to set this state")
            checked: enabled

            enabled: {
                if (chosenRoomDelegate.visible) {
                    let room = root.connection.room(chosenRoomDelegate.roomId);
                    if (room) {
                        if (room.canSendState("m.space.parent")) {
                            return true;
                        }
                    }
                }
                return false;
            }
        }

        FormCard.FormDelegateSeparator {
            above: existingOfficialCheck
            below: makeCanonicalCheck
        }

        FormCard.FormCheckDelegate {
            id: makeCanonicalCheck
            text: i18nc("@option:check The canonical parent is the default one if a room has multiple parent spaces.", "Make this space the canonical parent")
            description: i18nc("@info:description", "The canonical parent is the default one if a room has multiple parent spaces.")
            checked: enabled

            enabled: existingOfficialCheck.enabled
        }
    }
}
