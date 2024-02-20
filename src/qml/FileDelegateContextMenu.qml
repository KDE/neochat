// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import Qt.labs.platform

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.config

/**
 * @brief The menu for media messages.
 *
 * This component just overloads the actions and nested actions of the base menu
 * to what is required for a media item.
 *
 * @sa DelegateContextMenu
 */
DelegateContextMenu {
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
                currentRoom.openEventMediaExternally(root.eventId);
            }
        },
        Kirigami.Action {
            text: i18n("Save As")
            icon.name: "document-save"
            onTriggered: {
                var dialog = saveAsDialog.createObject(QQC2.ApplicationWindow.overlay);
                dialog.open();
                dialog.currentFile = dialog.folder + "/" + currentRoom.fileNameToDownload(eventId);
            }
        },
        DelegateContextMenu.ReplyMessageAction {},
        Kirigami.Action {
            text: i18n("Copy")
            icon.name: "edit-copy"
            onTriggered: {
                currentRoom.copyEventMedia(root.eventId);
            }
        },
        Kirigami.Action {
            visible: author.id === currentRoom.localUser.id || currentRoom.canSendState("redact")
            text: i18n("Remove")
            icon.name: "edit-delete-remove"
            icon.color: "red"
            onTriggered: applicationWindow().pageStack.pushDialogLayer("qrc:/org/kde/neochat/qml/RemoveSheet.qml", {
                room: currentRoom,
                eventId: eventId
            }, {
                title: i18nc("@title", "Remove Message"),
                width: Kirigami.Units.gridUnit * 25
            })
        },
        DelegateContextMenu.ReportMessageAction {},
        DelegateContextMenu.ViewSourceAction {}
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
            property string filename: StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId)

            doBeforeSharing: () => {
                currentRoom.downloadFile(eventId, filename);
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
                Config.lastSaveDirectory = folder;
                Config.save();
                currentRoom.downloadFile(eventId, currentFile);
            }
        }
    }
}
