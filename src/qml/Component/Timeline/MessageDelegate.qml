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

    function positionAt(x, y) {
        let point = label.mapFromItem(messageDelegate, x, y)
        return label.positionAt(point.x, point.y)
    }

    property alias selectedText: label.selectedText

    property bool isEmote: false
    onOpenContextMenu: openMessageContext(model, label.selectedText, Controller.plainText(label.textDocument))

    innerObject: ColumnLayout {
        Layout.maximumWidth: messageDelegate.contentMaxWidth
        RichLabel {
            id: label
            Layout.fillWidth: true
            isEmote: messageDelegate.isEmote
            isDelegate: true
        }
        Loader {
            id: linkPreviewLoader
            Layout.fillWidth: true
            active: !currentRoom.usesEncryption && model.display && model.display.includes("http")
            visible: Config.showLinkPreview && active
            sourceComponent: LinkPreviewDelegate {
                anchors.verticalCenter: parent.verticalCenter
            }
        }
    }
}
