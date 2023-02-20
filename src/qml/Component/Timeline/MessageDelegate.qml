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
    onOpenContextMenu: openMessageContext(model, label.selectedText, Controller.plainText(label.textDocument))

    innerObject: ColumnLayout {
        Layout.maximumWidth: messageDelegate.contentMaxWidth
        RichLabel {
            id: label
            Layout.fillWidth: true
            visible: currentRoom.chatBoxEditId !== model.eventId
            isEmote: messageDelegate.isEmote
        }
        Loader {
            Layout.fillWidth: true
            Layout.minimumHeight: item ? item.minimumHeight : -1
            Layout.preferredWidth: item ? item.preferredWidth : -1
            visible: currentRoom.chatBoxEditId === model.eventId
            active: visible
            sourceComponent: MessageEditComponent {
                room: currentRoom
                messageId: model.eventId
            }
        }
        LinkPreviewDelegate {
            Layout.fillWidth: true
            indicatorEnabled: messageDelegate.isVisibleInTimeline()
        }
    }
}
