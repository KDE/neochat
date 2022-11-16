// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.20 as Kirigami
import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: devtoolsPage

    property var room

    title: i18n("Room State - %1", room.displayName)

    ListView {
        anchors.fill: parent
        model: StateModel {
            room: devtoolsPage.room
        }

        delegate: Kirigami.BasicListItem {
            text: model.type
            subtitle: model.stateKey
            onClicked: applicationWindow().pageStack.pushDialogLayer('qrc:/MessageSourceSheet.qml', {
                sourceText: model.source
            }, {
                title: i18n("Event Source"),
                width: Kirigami.Units.gridUnit * 25
            });
        }
    }
}
