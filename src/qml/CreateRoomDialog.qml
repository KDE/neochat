// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as Components

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    property string parentId: ""

    property bool isSpace: false

    property bool showChildType: false

    property bool showCreateChoice: false

    required property NeoChatConnection connection

    signal addChild(string childId, bool setChildParent, bool canonical)
    signal newChild(string childName)

    title: isSpace ? i18nc("@title", "Create a Space") : i18nc("@title", "Create a Room")

    Component.onCompleted: roomNameField.forceActiveFocus()

    FormCard.FormHeader {
        title: root.isSpace ? i18n("New Space Information") : i18n("New Room Information")
    }
    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: roomTypeCombo
            property bool isInitialising: true

            visible: root.showChildType

            text: i18n("Select type")
            model: ListModel {
                id: roomTypeModel
            }
            textRole: "text"
            valueRole: "isSpace"

            Component.onCompleted: {
                currentIndex = indexOfValue(root.isSpace)
                roomTypeModel.append({"text":  i18n("Room"), "isSpace": false});
                roomTypeModel.append({"text":  i18n("Space"), "isSpace": true});
                roomTypeCombo.currentIndex = 0
                roomTypeCombo.isInitialising = false
            }
            onCurrentValueChanged: {
                if (!isInitialising) {
                    root.isSpace = currentValue
                }
            }
        }
        FormCard.FormTextFieldDelegate {
            id: roomNameField
            label: i18n("Name:")
            onAccepted: if (roomNameField.text.length > 0) roomTopicField.forceActiveFocus();
        }

        FormCard.FormTextFieldDelegate {
            id: roomTopicField
            label: i18n("Topic:")
            onAccepted: ok.clicked()
        }
        FormCard.FormCheckDelegate {
            id: newOfficialCheck
            visible: root.parentId.length > 0
            text: i18nc("@option:check As in make the space from which this dialog was created an official parent.", "Make this parent official")
            checked: true
        }
        FormCard.FormButtonDelegate {
            id: ok
            text: i18nc("@action:button", "Ok")
            enabled: roomNameField.text.length > 0
            onClicked: {
                if (root.isSpace) {
                    root.connection.createSpace(roomNameField.text, roomTopicField.text, root.parentId, newOfficialCheck.checked);
                } else {
                    root.connection.createRoom(roomNameField.text, roomTopicField.text, root.parentId, newOfficialCheck.checked);
                }
                root.newChild(roomNameField.text)
                root.closeDialog()
            }
        }
    }
    FormCard.FormHeader {
        visible: root.showChildType
        title: i18n("Select Existing Room")
    }
    FormCard.FormCard {
        visible: root.showChildType
        FormCard.FormButtonDelegate {
            visible: !chosenRoomDelegate.visible
            text: i18nc("@action:button", "Pick room")
            onClicked: {
                let dialog = pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/ExploreRoomsPage.qml", {connection: root.connection}, {title: i18nc("@title", "Explore Rooms")})
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
                let dialog = pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/ExploreRoomsPage.qml", {connection: root.connection}, {title: i18nc("@title", "Explore Rooms")})
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
            visible: root.parentId.length > 0
            text: i18nc("@option:check As in make the space from which this dialog was created an official parent.", "Make this parent official")
            description: enabled ? i18n("You have the required privilege level in the child to set this state") : i18n("You do not have a high enough privilege level in the child to set this state")
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
        FormCard.FormCheckDelegate {
            id: makeCanonicalCheck
            text: i18nc("@option:check The canonical parent is the default one if a room has multiple parent spaces.", "Make this space the canonical parent")
            checked: enabled

            enabled: existingOfficialCheck.enabled
        }
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Ok")
            enabled: chosenRoomDelegate.visible
            onClicked: {
                root.addChild(chosenRoomDelegate.roomId, existingOfficialCheck.checked, makeCanonicalCheck.checked);
                root.closeDialog();
            }
        }
    }
}
