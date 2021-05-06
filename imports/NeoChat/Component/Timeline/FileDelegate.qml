// SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kcoreaddons 1.0 as KCA

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu.Timeline 1.0

RowLayout {
    id: root
    property bool openOnFinished: false
    readonly property bool downloaded: progressInfo && progressInfo.completed

    Layout.margins: Kirigami.Units.largeSpacing

    spacing: Kirigami.Units.largeSpacing

    onDownloadedChanged: if (downloaded && openOnFinished) {
        openSavedFile();
    }

    ToolButton {
        icon.name: progressInfo.completed ? "document-open" : "document-save"
        onClicked: progressInfo.completed ? openSavedFile() : saveFileAs()
    }

    ColumnLayout {
        Kirigami.Heading {
            Layout.fillWidth: true
            level: 4
            text: model.display
            wrapMode: Label.Wrap
        }

        Label {
            Layout.fillWidth: true
            text: !progressInfo.completed && progressInfo.active ? (KCA.Format.formatByteSize(progressInfo.progress) + "/" + KCA.Format.formatByteSize(progressInfo.total)) : KCA.Format.formatByteSize(content.info ? content.info.size : 0)
            color: Kirigami.Theme.disabledTextColor
            wrapMode: Label.Wrap
        }
    }

    Component {
        id: fileDialog

        FileDialog {
            fileMode: FileDialog.SaveFile
            folder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)
            onAccepted: {
                currentRoom.downloadFile(eventId, file)
            }
        }
    }

    function saveFileAs() {
        var dialog = fileDialog.createObject(ApplicationWindow.overlay)
        dialog.open()
        dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(eventId)
    }

    function downloadAndOpen() {
        if (downloaded) {
            openSavedFile();
        } else {
            openOnFinished = true;
            currentRoom.downloadFile(eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/"
                + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId));
        }
    }

    function openSavedFile() {
        if (Qt.openUrlExternally(progressInfo.localPath)) return;
        if (Qt.openUrlExternally(progressInfo.localDir)) return;
    }
}
