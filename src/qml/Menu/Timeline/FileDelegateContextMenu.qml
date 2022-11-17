// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

MessageDelegateContextMenu {
    id: root

    signal closeFullscreen

    required property var file
    required property var progressInfo
    required property string mimeType

    property list<Kirigami.Action> actions: [
        Kirigami.Action {
            text: i18n("Open Externally")
            icon.name: "document-open"
            onTriggered: {
                if (file.downloaded) {
                    if (!UrlHelper.openUrl(progressInfo.localPath)) {
                        UrlHelper.openUrl(progressInfo.localDir);
                    }
                } else {
                    file.onDownloadedChanged.connect(function() {
                        if (!UrlHelper.openUrl(progressInfo.localPath)) {
                            UrlHelper.openUrl(progressInfo.localDir);
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
                var dialog = saveAsDialog.createObject(QQC2.ApplicationWindow.overlay)
                dialog.open()
                dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(eventId)
            }
        },
        Kirigami.Action {
            text: i18n("Reply")
            icon.name: "mail-replied-symbolic"
            onTriggered: {
                currentRoom.chatBoxReplyId = eventId
                currentRoom.chatBoxEditId = ""
                root.closeFullscreen()
            }
        },
        Kirigami.Action {
            visible: author.id === currentRoom.localUser.id || currentRoom.canSendState("redact")
            text: i18n("Remove")
            icon.name: "edit-delete-remove"
            icon.color: "red"
            onTriggered: {
                currentRoom.redactEvent(eventId);
                root.closeFullscreen()
            }
        },
        Kirigami.Action {
            text: i18nc("@action:button 'Report' as in 'Report this event to the administrators'", "Report")
            icon.name: "dialog-warning-symbolic"
            visible: author.id !== currentRoom.localUser.id
            onTriggered: applicationWindow().pageStack.pushDialogLayer("qrc:/ReportSheet.qml", {room: currentRoom, eventId: eventId}, {
                title: i18nc("@title", "Report Message"),
                width: Kirigami.Units.gridUnit * 25
            })
        },
        Kirigami.Action {
            text: i18n("View Source")
            icon.name: "code-context"
            onTriggered: {
                applicationWindow().pageStack.pushDialogLayer('qrc:/MessageSourceSheet.qml', {
                    sourceText: root.source
                }, {
                    title: i18n("Message Source"),
                    width: Kirigami.Units.gridUnit * 25
                });
                root.closeFullscreen()
            }
        }
    ]

    property list<Kirigami.Action> nestedActions: [
        ShareAction {
            id: shareAction
            inputData: {
                'urls': [],
                'mimeType': [mimeType]
            }
            property string filename: StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId);

            doBeforeSharing: () => {
                currentRoom.downloadFile(eventId, filename)
            }
            Component.onCompleted: {
                shareAction.inputData = {
                    urls: [filename],
                    mimeType: [mimeType]
                };
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
