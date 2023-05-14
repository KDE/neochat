// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm
import org.kde.kitemmodels 1.0

import org.kde.neochat 1.0

ColumnLayout {
    MobileForm.FormCard {
        Layout.fillWidth: true
        contentItem: ColumnLayout {
            spacing: 0
            MobileForm.FormComboBoxDelegate {
                text: i18n("Room")
                textRole: "displayName"
                valueRole: "id"
                model: RoomListModel {
                    id: roomListModel
                    connection: Controller.activeConnection
                }
                Component.onCompleted: currentIndex = indexOfValue(room.id)
                onCurrentValueChanged: room = roomListModel.roomByAliasOrId(currentValue)
            }
            MobileForm.FormCheckDelegate {
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
            MobileForm.FormCheckDelegate {
                id: roomAccoutnDataVisibleCheck
                text: i18n("Show room account data")
                checked: false
            }
        }
    }
    MobileForm.FormCard {
        Layout.fillWidth: true
        visible: roomAccoutnDataVisibleCheck.checked
        contentItem: ColumnLayout {
            spacing: 0
            MobileForm.FormCardHeader {
                title: i18n("Room Account Data for %1 - %2", room.displayName, room.id)
            }

            Repeater {
                model: room.accountDataEventTypes
                delegate: MobileForm.FormTextDelegate {
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
    }
    MobileForm.FormCard {
        Layout.fillWidth: true
        Layout.fillHeight: true
        contentItem: ColumnLayout {
            spacing: 0
            MobileForm.FormCardHeader {
                id: stateEventListHeader
                title: i18n("Room State for %1", room.displayName)
                subtitle: room.id
            }
            QQC2.ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true

                // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
                QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

                ListView {
                    id: stateEventListView
                    clip: true

                    model: StateFilterModel {
                        id: stateEventFilterModel
                        sourceModel: StateModel {
                            id: stateModel
                            room: devtoolsPage.room
                        }
                    }

                    delegate: MobileForm.FormTextDelegate {
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
}
