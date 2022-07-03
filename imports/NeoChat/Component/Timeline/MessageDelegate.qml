// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import Qt.labs.qmlmodels 1.0
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

TimelineContainer {
    id: messageDelegate

    property bool isEmote: false

    onReplyClicked: ListView.view.goToEvent(eventID)
    hoverComponent: hoverActions

    innerObject: RichLabel {
        isEmote: messageDelegate.isEmote
        Layout.maximumWidth: messageDelegate.bubbleMaxWidth

        TapHandler {
            acceptedButtons: Qt.RightButton
            onTapped: openMessageContext(model, parent.selectedText, Controller.plainText(parent.textDocument))
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onLongPressed: openMessageContext(model, parent.selectedText, Controller.plainText(parent.textDocument))
        }
    }
}
