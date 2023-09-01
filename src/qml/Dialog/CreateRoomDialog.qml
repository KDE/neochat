// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    parent: applicationWindow().overlay

    title: i18n("Create a Room")

    Kirigami.FormLayout {
        QQC2.TextField {
            id: roomNameField
            Kirigami.FormData.label: i18n("Room name:")
            onAccepted: roomTopicField.forceActiveFocus();
        }

        QQC2.TextField {
            id: roomTopicField
            Kirigami.FormData.label: i18n("Room topic:")
            onAccepted: okButton.forceActiveFocus();
        }

        QQC2.Button {
            id: okButton

            text: i18nc("@action:button", "Ok")
            onClicked: {
                Controller.createRoom(roomNameField.text, roomTopicField.text);
                root.close();
                root.destroy();
            }
        }
    }
}
