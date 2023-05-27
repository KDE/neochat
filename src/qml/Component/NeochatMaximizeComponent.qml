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

    required property string eventId

    required property var time

    required property var author

    required property int delegateType

    required property string plainText

    required property string caption

    required property var mediaInfo

    required property var progressInfo

    required property var mimeType

    required property var source

    property list<Components.AlbumModelItem> items: [
        Components.AlbumModelItem {
            type: root.delegateType === MessageEventModel.Image || root.delegateType === MessageEventModel.Sticker ? Components.AlbumModelItem.Image : Components.AlbumModelItem.Video
            source: root.delegateType === MessageEventModel.Video ? root.progressInfo.localPath : root.mediaInfo.source
            tempSource: root.mediaInfo.tempInfo.source
            caption: root.caption
            sourceWidth: root.mediaInfo.width
            sourceHeight: root.mediaInfo.height
        }
    ]

    model: items
    initialIndex: 0

    leading: RowLayout {
        Kirigami.Avatar {
            id: userAvatar
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: Kirigami.Units.iconSizes.medium

            name: root.author.name ?? root.author.displayName
            source: root.author.avatarSource
            color: root.author.color
        }
        ColumnLayout {
            spacing: 0
            QQC2.Label {
                id: userLabel
                text: root.author.name ?? root.author.displayName
                color: root.author.color
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            QQC2.Label {
                id: dateTimeLabel
                text: root.time.toLocaleString(Qt.locale(), Locale.ShortFormat)
                color: Kirigami.Theme.disabledTextColor
                elide: Text.ElideRight
            }
        }
    }
    onItemRightClicked: {
        const contextMenu = fileDelegateContextMenu.createObject(parent, {
            author: root.author,
            message: root.plainText,
            eventId: root.eventId,
            source: root.source,
            file: parent,
            mimeType: root.mimeType,
            progressInfo: root.progressInfo,
            plainMessage: root.plainText,
        });
        contextMenu.closeFullscreen.connect(root.close)
        contextMenu.open();
    }
    onSaveItem: {
        var dialog = saveAsDialog.createObject(QQC2.ApplicationWindow.overlay)
        dialog.open()
        dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(root.eventId)
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
                currentRoom.downloadFile(rooteventId, currentFile)
            }
        }
    }
}
