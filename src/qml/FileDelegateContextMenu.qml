// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtCore as Core
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Dialogs as Dialogs

import org.kde.kirigami as Kirigami

import org.kde.neochat

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

    // Web search isn't useful for images
    enableWebSearch: false

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
                var dialog = saveAsDialog.createObject(QQC2.Overlay.overlay);
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
            visible: author.id === currentRoom.localMember.id || currentRoom.canSendState("redact")
            text: i18n("Remove")
            icon.name: "edit-delete-remove"
            icon.color: "red"
            onTriggered: {
                let dialog = applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                    title: i18nc("@title:dialog", "Remove Message"),
                    placeholder: i18nc("@info:placeholder", "Reason for removing this message"),
                    actionText: i18nc("@action:button 'Remove' as in 'Remove this message'", "Remove"),
                    icon: "delete"
                }, {
                    title: i18nc("@title:dialog", "Remove Message"),
                    width: Kirigami.Units.gridUnit * 25
                });
                dialog.accepted.connect(reason => {
                    currentRoom.redactEvent(root.eventId, reason);
                });
            }
        },
        DelegateContextMenu.ReportMessageAction {},
        DelegateContextMenu.ShowUserAction {},
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
                "urls": [filename],
                "mimeType": [root.mimeType]
            }
            room: currentRoom
            eventId: root.eventId
            property string filename: Core.StandardPaths.writableLocation(Core.StandardPaths.CacheLocation) + "/" + eventId.replace(":", "_").replace("/", "_").replace("+", "_") + currentRoom.fileNameToDownload(eventId)
        }
    ]

    Component {
        id: saveAsDialog
        Dialogs.FileDialog {
            fileMode: Dialogs.FileDialog.SaveFile
            currentFolder: NeoChatConfig.lastSaveDirectory.length > 0 ? NeoChatConfig.lastSaveDirectory : Core.StandardPaths.writableLocation(Core.StandardPaths.DownloadLocation)
            onAccepted: {
                if (!selectedFile) {
                    return;
                }
                NeoChatConfig.lastSaveDirectory = currentFolder;
                NeoChatConfig.save();
                currentRoom.downloadFile(eventId, selectedFile);
            }
        }
    }
}
