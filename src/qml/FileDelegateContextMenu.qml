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

    DelegateContextMenu.ReplyMessageAction {}

    Kirigami.Action {
        separator: true
    }

    QQC2.Action {
        text: i18nc("@action:inmenu", "Open Image")
        icon.name: "document-open"
        onTriggered: {
            currentRoom.openEventMediaExternally(root.eventId);
        }
    }

    QQC2.Action {
        text: i18nc("@action:inmenu", "Save Image…")
        icon.name: "document-save"
        onTriggered: {
            var dialog = saveAsDialog.createObject(QQC2.Overlay.overlay);
            dialog.selectedFile = currentRoom.fileNameToDownload(eventId);
            dialog.open();
        }
    }

    QQC2.Action {
        text: i18nc("@action:inmenu", "Copy Image")
        icon.name: "edit-copy"
        onTriggered: {
            currentRoom.copyEventMedia(root.eventId);
        }
    }

    Kirigami.Action {
        separator: true
    }

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
    }

    DelegateContextMenu.ReportMessageAction {}

    DelegateContextMenu.ShowUserAction {}

    Kirigami.Action {
        separator: true
        visible: viewSourceAction.visible
    }

    DelegateContextMenu.ViewSourceAction {
        id: viewSourceAction
    }

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

    readonly property Component saveAsDialog: Dialogs.FileDialog {
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
