/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import NeoChat.Component 1.0
import NeoChat.Setting 1.0

Dialog {
    anchors.centerIn: parent
    width: 360

    id: root

    title: "Enter Font Family"

    contentItem: AutoTextField {
        Layout.fillWidth: true

        id:fontFamilyField

        text: MSettings.fontFamily
        placeholderText: "Font Family"
    }

    standardButtons: Dialog.Ok | Dialog.Cancel

    onAccepted: MSettings.fontFamily = fontFamilyField.text

    onClosed: destroy()
}
