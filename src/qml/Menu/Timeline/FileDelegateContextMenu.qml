// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

/**
 * @brief The menu for media messages.
 *
 * This component just overloads the actions and nested actions of the base menu
 * to what is required for a media item.
 *
 * @sa MessageDelegateContextMenu
 */
MessageDelegateContextMenu {
    id: root

    /**
     * @brief The MIME type of the media.
     */
    property string mimeType

    /**
     * @brief Progress info when downloading files.
     *
     * @sa Quotient::FileTransferInfo
     */
    required property var progressInfo

    /**
     * @brief The main list of menu item actions.
     *
     * Each action will be instantiated as a single line in the menu.
     */
    property list<Kirigami.Action> actions: [
        Kirigami.Action {
            text: i18n("Open Externally")
            icon.name: "document-open"
            onTriggered: {
                currentRoom.openEventMediaExternally(root.eventId)
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
                RoomManager.requestFullScreenClose()
            }
        },
        Kirigami.Action {
            text: i18n("Copy")
            icon.name: "edit-copy"
            onTriggered: {
                currentRoom.copyEventMedia(root.eventId)
            }
        },
        Kirigami.Action {
            visible: author.id === currentRoom.localUser.id || currentRoom.canSendState("redact")
            text: i18n("Remove")
            icon.name: "edit-delete-remove"
            icon.color: "red"
            onTriggered: applicationWindow().pageStack.pushDialogLayer("qrc:/RemoveSheet.qml", {room: currentRoom, eventId: eventId}, {
                title: i18nc("@title", "Remove Message"),
                width: Kirigami.Units.gridUnit * 25
            })
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
            onTriggered: RoomManager.viewEventSource(root.eventId)
        }
    ]

    /**
     * @brief The list of menu item actions that have sub-actions.
     *
     * Each action will be instantiated as a single line that opens a sub menu.
     */
    property list<Kirigami.Action> nestedActions: [
        ShareAction {
            id: shareAction
            inputData: {
                'urls': [],
                'mimeType': [root.mimeType]
            }
            property string filename: StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId);

            doBeforeSharing: () => {
                currentRoom.downloadFile(eventId, filename)
            }
            Component.onCompleted: {
                shareAction.inputData = {
                    urls: [filename],
                    mimeType: [root.mimeType]
                };
            }
        }
    ]

    Component {
        id: saveAsDialog
        FileDialog {
            fileMode: FileDialog.SaveFile
            folder: Config.lastSaveDirectory.length > 0 ? Config.lastSaveDirectory : StandardPaths.writableLocation(StandardPaths.DownloadLocation)
            onAccepted: {
                if (!currentFile) {
                    return;
                }
                Config.lastSaveDirectory = folder
                Config.save()
                currentRoom.downloadFile(eventId, currentFile)
            }
        }
    }
}
