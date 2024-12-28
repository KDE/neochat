// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtCore as Core
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs as Dialogs
import QtMultimedia

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as Components

import org.kde.neochat

Components.AlbumMaximizeComponent {
    id: root

    /**
     * @brief The current room that user is viewing.
     */
    required property NeoChatRoom currentRoom

    readonly property string currentEventId: model.data(model.index(content.currentIndex, 0), TimelineMessageModel.EventIdRole)

    readonly property var currentAuthor: model.data(model.index(content.currentIndex, 0), TimelineMessageModel.AuthorRole)

    readonly property var currentTime: model.data(model.index(content.currentIndex, 0), TimelineMessageModel.TimeRole)

    readonly property var currentProgressInfo: model.data(model.index(content.currentIndex, 0), TimelineMessageModel.ProgressInfoRole)

    /**
     * @brief Whether the delegate is part of a thread timeline.
     */
    property bool isThread: false

    downloadAction: Components.DownloadAction {
        id: downloadAction
        onTriggered: {
            currentRoom.downloadFile(root.currentEventId, Core.StandardPaths.writableLocation(Core.StandardPaths.CacheLocation) + "/" + root.currentEventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(root.currentEventId));
        }
    }

    playAction: Kirigami.Action {
        onTriggered: {
            MediaManager.startPlayback();
            currentItem.play();
        }
    }

    Connections {
        target: MediaManager
        function onPlaybackStarted() {
            if (currentItem.playbackState === MediaPlayer.PlayingState) {
                currentItem.pause();
            }
        }
    }

    Connections {
        target: currentRoom

        function onFileTransferProgress(id, progress, total) {
            if (id == root.currentEventId) {
                downloadAction.progress = progress / total * 100.0;
            }
        }
    }

    Connections {
        target: content

        function onCurrentIndexChanged() {
            downloadAction.progress = currentProgressInfo.progress / currentProgressInfo.total * 100.0;
        }
    }

    leading: RowLayout {
        Components.Avatar {
            id: userAvatar
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: Kirigami.Units.iconSizes.medium

            name: root.currentAuthor.name ?? root.currentAuthor.displayName
            source: root.currentAuthor.avatarUrl
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

    onOpened: forceActiveFocus()

    onItemRightClicked: RoomManager.viewEventMenu(root.currentEventId, root.currentRoom)

    onSaveItem: {
        var dialog = saveAsDialog.createObject(QQC2.Overlay.overlay);
        dialog.selectedFile = currentRoom.fileNameToDownload(root.currentEventId);
        dialog.open();
    }

    Connections {
        target: RoomManager
        function onCloseFullScreen() {
            root.close();
        }
    }

    Component {
        id: saveAsDialog
        Dialogs.FileDialog {
            fileMode: Dialogs.FileDialog.SaveFile
            currentFolder: root.saveFolder
            onAccepted: {
                NeoChatConfig.lastSaveDirectory = currentFolder;
                NeoChatConfig.save();
                if (!selectedFile) {
                    return;
                }
                currentRoom.downloadFile(root.currentEventId, selectedFile);
            }
        }
    }
}
