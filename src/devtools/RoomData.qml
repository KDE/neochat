// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

ColumnLayout {
    id: root

    required property NeoChatRoom room
    required property NeoChatConnection connection

    FormCard.FormHeader {
        title: i18nc("@title", "Choose Room")
    }
    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: root.room?.displayNameForHtml ?? i18nc("@info", "No room selected")
            description: i18nc("@info", "Click to choose a room");

            onClicked: {
                let dialog = (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ChooseRoomDialog'), {
                    connection: root.connection,
                }, {
                    title: i18nc("@title:dialog", "Choose Room"),
                    width: Kirigami.Units.gridUnit * 24
                });
                dialog.chosen.connect(id => root.room = root.connection.room(id))
            }
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormTextDelegate {
            visible: root.room
            text: i18n("Room Id: %1", root.room.id)
        }
        FormCard.FormDelegateSeparator {}
        FormCard.FormCheckDelegate {
            visible: root.room
            text: i18nc("@option:check Toggle for whether this room is considered a direct chat or not", "Direct Chat")
            checked: root.room.isDirectChat()
            enabled: root.room.totalMemberCount === 2
            onCheckedChanged: {
                if (checked) {
                    root.room.markAsDirectChat();
                } else {
                    root.connection.removeFromDirectChats(root.room.id);
                }
            }
        }
    }
    FormCard.FormHeader {
        title: i18n("Room Account Data")
        visible: roomAccountData.count > 0
    }
    FormCard.FormCard {
        visible: roomAccountData.count > 0
        Repeater {
            id: roomAccountData
            model: root.room.accountDataEventTypes
            delegate: FormCard.FormButtonDelegate {
                required property string modelData
                text: modelData
                onClicked: (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'MessageSourceSheet'), {
                    sourceText: root.room.roomAcountDataJson(text)
                }, {
                    title: i18n("Event Source"),
                    width: Kirigami.Units.gridUnit * 25
                })
            }
        }
    }
    FormCard.FormHeader {
        id: stateEventListHeader
        title: i18n("Room State")
        visible: roomState.count > 0
    }
    FormCard.FormCard {
        visible: roomState.count > 0
        Repeater {
            id: roomState
            model: StateModel {
                id: stateModel
                room: root.room
            }

            delegate: FormCard.FormButtonDelegate {
                required property string type
                required property int eventCount
                required property string stateKey
                text: type
                description: i18ncp("'Event' being some JSON data, not something physically happening.", "%1 event of this type", "%1 events of this type", eventCount)
                onClicked: {
                    if (eventCount === 1) {
                        root.openEventSource(type, stateKey);
                    } else {
                        (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat.devtools', 'StateKeys'), {
                            room: root.room,
                            eventType: type
                        }, {
                            title: i18nc("'Event' being some JSON data, not something physically happening.", "Event Information")
                        });
                    }
                }
            }
        }
    }
    function openEventSource(type: string, stateKey: string): void {
        onClicked: (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'MessageSourceSheet'), {
            model: stateModel,
            allowEdit: true,
            room: root.room,
            type: type,
            stateKey: stateKey,
        }, {
            title: i18n("Event Source"),
            width: Kirigami.Units.gridUnit * 25
        });
    }
}
