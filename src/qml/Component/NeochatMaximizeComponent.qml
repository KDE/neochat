// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1 as Platform

import org.kde.kirigami 2.13 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as Components

import org.kde.neochat 1.0

Components.AlbumMaximizeComponent {
    id: root

    readonly property string currentEventId: model.data(model.index(content.currentIndex, 0), MessageEventModel.EventIdRole)

    readonly property var currentAuthor: model.data(model.index(content.currentIndex, 0), MessageEventModel.AuthorRole)

    readonly property var currentTime: model.data(model.index(content.currentIndex, 0), MessageEventModel.TimeRole)

    readonly property string currentPlainText: model.data(model.index(content.currentIndex, 0), MessageEventModel.PlainText)

    readonly property var currentMimeType: model.data(model.index(content.currentIndex, 0), MessageEventModel.MimeTypeRole)

    readonly property var currentProgressInfo: model.data(model.index(content.currentIndex, 0), MessageEventModel.ProgressInfoRole)

    readonly property var currentJsonSource: model.data(model.index(content.currentIndex, 0), MessageEventModel.SourceRole)

    downloadAction: Components.DownloadAction {
        id: downloadAction
        onTriggered: {
            currentRoom.downloadFile(root.currentEventId, Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/" + root.currentEventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(root.currentEventId))
        }
    }

    Connections {
        target: currentRoom

        function onFileTransferProgress(id, progress, total) {
            if (id == root.currentEventId) {
                downloadAction.progress = progress / total * 100.0
            }
        }
    }

    Connections {
        target: content

        function onCurrentIndexChanged() {
            downloadAction.progress = currentProgressInfo.progress / currentProgressInfo.total * 100.0
        }
    }

    leading: RowLayout {
        Components.Avatar {
            id: userAvatar
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: Kirigami.Units.iconSizes.medium

            name: root.currentAuthor.name ?? root.currentAuthor.displayName
            source: root.currentAuthor.avatarSource
            color: root.currentAuthor.color
        }
        ColumnLayout {
            spacing: 0
            QQC2.Label {
                id: userLabel

                text: root.currentAuthor.name ?? root.currentAuthor.displayName
                color: root.currentAuthor.color
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            QQC2.Label {
                id: dateTimeLabel
                text: root.currentTime.toLocaleString(Qt.locale(), Locale.ShortFormat)
                color: Kirigami.Theme.disabledTextColor
                elide: Text.ElideRight
            }
        }
    }
    onItemRightClicked: {
        const contextMenu = fileDelegateContextMenu.createObject(parent, {
            author: root.currentAuthor,
            eventId: root.currentEventId,
            source: root.currentJsonSource,
            file: parent,
            mimeType: root.currentMimeType,
            progressInfo: root.currentProgressInfo,
            plainText: root.currentPlainText
        });
        contextMenu.closeFullscreen.connect(root.close)
        contextMenu.open();
    }
    onSaveItem: {
        var dialog = saveAsDialog.createObject(QQC2.ApplicationWindow.overlay)
        dialog.open()
        dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(root.currentEventId)
    }

    Component {
        id: saveAsDialog
        Platform.FileDialog {
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
