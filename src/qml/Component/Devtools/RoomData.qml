// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

FormCard.FormCardPage {
    id: root

    required property var room

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.gridUnit

        FormCard.FormComboBoxDelegate {
            id: roomChooser
            text: i18n("Room")
            textRole: "displayName"
            valueRole: "roomId"
            model: RoomListModel {
                id: roomListModel
                connection: Controller.activeConnection
            }
            onCurrentValueChanged: room = roomListModel.roomByAliasOrId(currentValue)
            Component.onCompleted: currentIndex = indexOfValue(room.id)
        }

        FormCard.FormDelegateSeparator { above: showRoomMember }

        FormCard.FormCheckDelegate {
            id: showRoomMember
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

        FormCard.FormDelegateSeparator { above: roomAccoutnDataVisibleCheck; below: showRoomMember }

        FormCard.FormCheckDelegate {
            id: roomAccoutnDataVisibleCheck
            text: i18n("Show room account data")
            checked: false
        }

        FormCard.FormDelegateSeparator { below: roomAccoutnDataVisibleCheck }

        FormCard.FormTextDelegate {
            text: i18n("Room id")
            description: room.id
        }
    }

    FormCard.FormHeader {
        title: i18n("Room Account Data for %1", room.displayName)
        visible: roomAccoutnDataVisibleCheck.checked
    }

    FormCard.FormCard {
        visible: roomAccoutnDataVisibleCheck.checked

        Repeater {
            model: room.accountDataEventTypes
            delegate: FormCard.FormTextDelegate {
                text: modelData
                onClicked: applicationWindow().pageStack.pushDialogLayer("qrc:/MessageSourceSheet.qml", {
                    "sourceText": room.roomAcountDataJson(text)
                }, {
                    "title": i18n("Event Source"),
                    "width": Kirigami.Units.gridUnit * 25
                })
            }
        }
    }

    FormCard.FormHeader {
        id: stateEventListHeader
        title: i18n("Room State for %1", room.displayName)
    }

    FormCard.FormCard {
        Layout.fillHeight: true

        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.fillHeight: true
            Layout.maximumHeight: Kirigami.Units.gridUnit * 20

            // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
            QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

            ListView {
                id: stateEventListView
                clip: true

                model: StateFilterModel {
                    id: stateEventFilterModel
                    sourceModel: StateModel {
                        id: stateModel
                        room: root.room
                    }
                }

                delegate: FormCard.FormTextDelegate {
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
}
