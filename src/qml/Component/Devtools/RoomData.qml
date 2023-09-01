// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

ColumnLayout {
    id: root

    required property NeoChatRoom room

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
                connection: Controller.activeConnection
            }
            currentIndex: -1
            Component.onCompleted: currentIndex = roomListModel.rowForRoom(root.room)
            onCurrentValueChanged: root.room = roomListModel.roomByAliasOrId(roomComboBox.currentValue)
        }
        FormCard.FormTextDelegate {
            text: i18n("Room Id: %1", root.room.id)
        }
        FormCard.FormCheckDelegate {
            text: i18n("Show m.room.member events")
            checked: true
            onToggled: {
                if (checked) {
                    stateEventFilterModel.removeStateEventTypeFiltered("m.room.member");
                } else {
                    stateEventFilterModel.addStateEventTypeFiltered("m.room.member");
                }
            }
        }
        FormCard.FormCheckDelegate {
            id: roomAccountDataVisibleCheck
            text: i18n("Show room account data")
            checked: false
        }
    }
    FormCard.FormHeader {
        visible: roomAccountDataVisibleCheck.checked
        title: i18n("Room Account Data")
    }
    FormCard.FormCard {
        visible: roomAccountDataVisibleCheck.checked
        Repeater {
            model: root.room.accountDataEventTypes
            delegate: FormCard.FormButtonDelegate {
                text: modelData
                onClicked: applicationWindow().pageStack.pushDialogLayer("qrc:/MessageSourceSheet.qml", {
                    "sourceText": root.room.roomAcountDataJson(text)
                }, {
                    "title": i18n("Event Source"),
                    "width": Kirigami.Units.gridUnit * 25
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
            model: StateFilterModel {
                id: stateEventFilterModel
                sourceModel: StateModel {
                    id: stateModel
                    room: root.room
                }
            }

            delegate: FormCard.FormButtonDelegate {
                text: model.type
                description: model.stateKey
                onClicked: applicationWindow().pageStack.pushDialogLayer('qrc:/MessageSourceSheet.qml', {
                    sourceText: stateModel.stateEventJson(stateEventFilterModel.mapToSource(stateEventFilterModel.index(model.index, 0)))
                }, {
                    title: i18n("Event Source"),
                    width: Kirigami.Units.gridUnit * 25
                });
            }
        }
    }
}
