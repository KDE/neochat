// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitemmodels

import org.kde.neochat

ColumnLayout {
    id: root

    required property NeoChatRoom room
    required property NeoChatConnection connection

    FormCard.FormHeader {
        title: i18nc("@title", "Choose Room")
    }
    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: roomComboBox
            text: i18n("Room")
            textRole: "displayName"
            valueRole: "roomId"
            model: RoomListModel {
                id: roomListModel
                connection: root.connection
            }
            currentIndex: -1
            Component.onCompleted: currentIndex = roomListModel.rowForRoom(root.room)
            onCurrentValueChanged: root.room = roomListModel.roomByAliasOrId(roomComboBox.currentValue)
        }
        FormCard.FormTextDelegate {
            text: i18n("Room Id: %1", root.room.id)
        }
    }
    FormCard.FormHeader {
        title: i18n("Room Account Data")
    }
    FormCard.FormCard {
        Repeater {
            model: root.room.accountDataEventTypes
            delegate: FormCard.FormButtonDelegate {
                text: modelData
                onClicked: applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'MessageSourceSheet.qml'), {
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
    }
    FormCard.FormCard {
        Repeater {
            model: StateModel {
                id: stateModel
                room: root.room
            }

            delegate: FormCard.FormButtonDelegate {
                text: model.type
                description: i18ncp("'Event' being some JSON data, not something physically happening.", "%1 event of this type", "%1 events of this type", model.eventCount)
                onClicked: {
                    if (model.eventCount === 1) {
                        onClicked: applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'MessageSourceSheet.qml'), {
                            sourceText: stateModel.stateEventJson(stateModel.index(model.index, 0))
                        }, {
                            title: i18n("Event Source"),
                            width: Kirigami.Units.gridUnit * 25
                        })
                    } else {
                        pageStack.pushDialogLayer(stateKeysComponent, {
                            room: root.room,
                            eventType: model.type
                        }, {
                            title: i18nc("'Event' being some JSON data, not something physically happening.", "Event Information")
                        });
                    }
                }
            }
        }
        Component {
            id: stateKeysComponent
            StateKeys {}
        }
    }
}
