// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.13 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as Components

import org.kde.neochat 1.0

Components.AlbumMaximizeComponent {
    id: root

    property var modelData

    property list<Components.AlbumModelItem> items: [
        Components.AlbumModelItem {
            type: root.modelData.delegateType === MessageEventModel.Image ? Components.AlbumModelItem.Image : Components.AlbumModelItem.Video
            source: root.modelData.delegateType === MessageEventModel.Video ? modelData.progressInfo.localPath : modelData.mediaUrl
            tempSource: modelData.content.info["xyz.amorgan.blurhash"] ? ("image://blurhash/" + modelData.content.info["xyz.amorgan.blurhash"]) : ""
            caption: modelData.display
        }
    ]

    model: items
    initialIndex: 0

    leading: RowLayout {
        Kirigami.Avatar {
            id: userAvatar
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: Kirigami.Units.iconSizes.medium

            name: modelData.author.name ?? modelData.author.displayName
            source: modelData.author.avatarMediaId ? ("image://mxc/" + modelData.author.avatarMediaId) : ""
            color: modelData.author.color
        }
        ColumnLayout {
            spacing: 0
            QQC2.Label {
                id: userLabel
                text: modelData.author.name ?? modelData.author.displayName
                color: modelData.author.color
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            QQC2.Label {
                id: dateTimeLabel
                text: modelData.time.toLocaleString(Qt.locale(), Locale.ShortFormat)
                color: Kirigami.Theme.disabledTextColor
                elide: Text.ElideRight
            }
        }
    }
    onItemRightClicked: {
        const contextMenu = fileDelegateContextMenu.createObject(parent, {
            author: modelData.author,
            message: modelData.message,
            eventId: modelData.eventId,
            source: modelData.source,
            file: parent,
            mimeType: modelData.mimeType,
            progressInfo: modelData.progressInfo,
            plainMessage: modelData.message,
        });
        contextMenu.closeFullscreen.connect(root.close)
        contextMenu.open();
    }
    onSaveItem: {
        var dialog = saveAsDialog.createObject(QQC2.ApplicationWindow.overlay)
        dialog.open()
        dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(modelData.eventId)
    }

    Component {
        id: saveAsDialog
        FileDialog {
            fileMode: FileDialog.SaveFile
            folder: root.saveFolder
            onAccepted: {
                Config.lastSaveDirectory = folder
                Config.save()
                if (!currentFile) {
                    return;
                }
                currentRoom.downloadFile(eventId, currentFile)
            }
        }
    }
}
