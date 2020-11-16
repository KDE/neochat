/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import NeoChat.Component 1.0

import org.kde.neochat 1.0

Dialog {
    anchors.centerIn: parent
    width: 360

    id: root

    title: i18n("Create a Room")

    contentItem: ColumnLayout {
        AutoTextField {
            Layout.fillWidth: true

            id: roomNameField

            placeholderText: i18n("Room Name")
        }

        AutoTextField {
            Layout.fillWidth: true

            id: roomTopicField

            placeholderText: i18n("Room Topic")
        }
    }

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: Controller.createRoom(Controller.activeConnection, roomNameField.text, roomTopicField.text)

    onClosed: destroy()
}
