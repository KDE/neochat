/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

import NeoChat.Component 1.0

import org.kde.neochat 1.0

Kirigami.OverlaySheet {
    id: root

    parent: applicationWindow().overlay

    header: Kirigami.Heading {
        text: i18n("Create a Room")
    }

    contentItem: Kirigami.FormLayout {
        TextField {
            id: roomNameField
            Kirigami.FormData.label: i18n("Room Name")
            onAccepted: roomTopixField.forceActiveFocus();
        }

        TextField {
            id: roomTopicField
            Kirigami.FormData.label: i18n("Room Topic")
            onAccepted: okButton.forceActiveFocus();
        }

        Button {
            id: okButton

            text: i18nc("@action:button", "Ok")
            onClicked: {
                roomManager.actionsHandler.createRoom(roomNameField.text, roomTopicField.text);
                root.close();
                root.destroy();
            }
        }
    }
}
