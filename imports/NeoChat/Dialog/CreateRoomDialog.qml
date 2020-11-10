/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import NeoChat.Component 2.0

import org.kde.neochat 0.1

Dialog {
    anchors.centerIn: parent
    width: 360

    id: root

    title: "Create a Room"

    contentItem: ColumnLayout {
        AutoTextField {
            Layout.fillWidth: true

            id: roomNameField

            placeholderText: "Room Name"
        }

        AutoTextField {
            Layout.fillWidth: true

            id: roomTopicField

            placeholderText: "Room Topic"
        }
    }

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: Controller.createRoom(Controller.connection, roomNameField.text, roomTopicField.text)

    onClosed: destroy()
}
