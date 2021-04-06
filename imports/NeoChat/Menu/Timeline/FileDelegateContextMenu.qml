// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Dialog 1.0
import NeoChat.Menu 1.0

MessageDelegateContextMenu {
    id: root

    required property var file
    required property var progressInfo

    property list<Kirigami.Action> actions: [
        Kirigami.Action {
            text: i18n("Open Externally")
            icon.name: "document-open"
            onTriggered: {
                if (file.downloaded) {
                    if (!Qt.openUrlExternally(progressInfo.localPath)) {
                        Qt.openUrlExternally(progressInfo.localDir);
                    }
                } else {
                    file.onDownloadedChanged.connect(function() {
                        if (!Qt.openUrlExternally(progressInfo.localPath)) {
                            Qt.openUrlExternally(progressInfo.localDir);
                        }
                    });
                    currentRoom.downloadFile(eventId, StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId))
                }
            }
        },
        Kirigami.Action {
            text: i18n("Save As")
            icon.name: "document-save"
            onTriggered: {
                var dialog = saveAsDialog.createObject(ApplicationWindow.overlay)
                dialog.open()
                dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(eventId)
            }
        },
        Kirigami.Action {
            text: i18n("Reply")
            icon.name: "mail-replied-symbolic"
            onTriggered: {
                ChatBoxHelper.replyToMessage(eventId, message, author);
            }
        },
        Kirigami.Action {
            visible: author.id === currentRoom.localUser.id || currentRoom.canSendState("redact")
            text: i18n("Remove")
            icon.name: "edit-delete-remove"
            icon.color: "red"
            onTriggered: {
                currentRoom.redactEvent(eventId);
            }
        },
        Kirigami.Action {
            text: i18n("View Source")
            icon.name: "code-context"
            onTriggered: {
                messageSourceSheet.createObject(root, {'sourceText': root.source}).open();
            }
        }
    ]
    Component {
        id: saveAsDialog
        FileDialog {
            fileMode: FileDialog.SaveFile
            folder: StandardPaths.writableLocation(StandardPaths.DownloadLocation)
            onAccepted: {
                if (!currentFile) {
                    return;
                }
                currentRoom.downloadFile(eventId, currentFile)
            }
        }
    }
}
