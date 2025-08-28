// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtCore as Core
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs as Dialogs

import Qt.labs.qmlmodels
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat

/**
 * @brief The menu for most message types.
 *
 * This menu supports showing a list of actions to be shown for a particular event
 * delegate in a message timeline. The menu supports both desktop and mobile menus
 * with different visuals appropriate to the platform.
 *
 * The menu supports both a list of main actions and the ability to define sub menus
 * using the nested action parameter.
 */
KirigamiComponents.ConvergentContextMenu {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The current connection for the account accessing the event.
     */
    required property NeoChatConnection connection

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The message author.
     *
     * A Quotient::RoomMember object.
     *
     * @sa Quotient::RoomMember
     */
    required property var author

    /**
     * @brief The delegate type of the message.
     */
    required property int messageComponentType

    /**
     * @brief The display text of the message as plain text.
     */
    required property string plainText

    /**
     * @brief The text the user currently has selected.
     */
    property string selectedText: ""

    /**
     * @brief The link the user has currently hovered.
     */
    property string hoveredLink: ""

    /**
     * @brief The HTML text of the event, if it is has one.
     */
    property string htmlText: ""

    /**
     * @brief Progress info when downloading files.
     *
     * @sa Quotient::FileTransferInfo
     */
    required property var progressInfo

    /**
     * @brief The MIME type of the media, or an empty string if the event does not have file content
     */
    property string mimeType

    /**
     * @brief Whether the event has file-based content. This includes images, videos, and other files
     */
    readonly property bool hasFileContent: mimeType.length > 0

    Kirigami.Action {
        id: emojiAction
        visible: root.messageComponentType === MessageComponentType.Other ? NeoChatConfig.relateAnyEvent : true

        displayComponent: RowLayout {
            visible: emojiAction.visible
            spacing: 0
            Layout.fillWidth: true
            Layout.preferredHeight: Kirigami.Units.gridUnit * 2.5
            Repeater {
                model: ["👍", "👎️", "😄", "🎉", "👀", "⋮"]
                delegate: Delegates.RoundedItemDelegate {
                    id: emojiDelegate
                    Layout.fillWidth: true
                    Layout.preferredWidth: Kirigami.Units.gridUnit * 2.5
                    Layout.fillHeight: true

                    contentItem: Kirigami.Heading {
                        id: emojiText
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        font.family: "emoji"
                        text: modelData
                    }

                    onClicked: {
                        if (emojiText.text === "⋮") {
                            var dialog = emojiDialog.createObject(emojiDelegate);
                            dialog.showStickers = false;
                            dialog.chosen.connect(emoji => {
                                root.room.toggleReaction(root.eventId, emoji);
                                root.close();
                            });
                            dialog.closed.connect(() => {
                                root.close();
                            });
                            dialog.open();
                            return;
                        }

                        currentRoom.toggleReaction(eventId, modelData);
                    }
                }
            }
            Component {
                id: emojiDialog

                EmojiDialog {
                    currentRoom: root.Message.room
                    showQuickReaction: true
                }
            }
        }
    }

    Kirigami.Action {
        separator: true
    }

    Kirigami.Action {
        visible: root.messageComponentType !== MessageComponentType.Other || NeoChatConfig.relateAnyEvent
        text: i18nc("@action:inmenu", "Reply")
        icon.name: "mail-replied-symbolic"
        onTriggered: {
            currentRoom.mainCache.replyId = eventId;
            currentRoom.editCache.editId = "";
            RoomManager.requestFullScreenClose();
        }
    }

    Kirigami.Action {
        visible: root.messageComponentType !== MessageComponentType.Other || NeoChatConfig.relateAnyEvent
        text: i18nc("@action:inmenu", "Reply in Thread")
        icon.name: "dialog-messages"
        onTriggered: {
            currentRoom.threadCache.replyId = "";
            currentRoom.threadCache.threadId = currentRoom.eventIsThreaded(root.eventId) ? currentRoom.rootIdForThread(root.eventId) : root.eventId;
            currentRoom.mainCache.clearRelations();
            currentRoom.editCache.clearRelations();
            RoomManager.requestFullScreenClose();
        }
    }

    Kirigami.Action {
        visible: !root.hasFileContent && root.author.isLocalMember && root.messageComponentType === MessageComponentType.Text
        text: i18n("Edit")
        icon.name: "document-edit"
        onTriggered: {
            currentRoom.editCache.editId = eventId;
            currentRoom.mainCache.replyId = "";
            currentRoom.mainCache.threadId = "";
        }
    }

    Kirigami.Action {
        visible: (author.isLocalMember || currentRoom.canSendState("redact")) && root.messageComponentType !== MessageComponentType.Other
        text: i18nc("@action:button", "Remove…")
        icon.name: "edit-delete-remove"
        icon.color: "red"
        onTriggered: {
            let dialog = (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
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

    Kirigami.Action {
        separator: true
    }

    Kirigami.Action {
        visible: root.messageComponentType !== MessageComponentType.Other
        text: i18nc("@action:inmenu As in 'Forward this message'", "Forward…")
        icon.name: "mail-forward-symbolic"
        onTriggered: {
            let page = (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ChooseRoomDialog'), {
                connection: root.connection
            }, {
                title: i18nc("@title", "Forward Message"),
                width: Kirigami.Units.gridUnit * 25
            });
            page.chosen.connect(function (targetRoomId) {
                root.connection.room(targetRoomId).postHtmlMessage(root.plainText, root.htmlText.length > 0 ? root.htmlText : root.plainText);
                page.closeDialog();
            });
        }
    }

    Kirigami.Action {
        visible: root.hasFileContent
        text: {
            if (root.messageComponentType === MessageComponentType.Image) {
                return i18nc("@action:inmenu", "Open Image");
            } else if (root.messageComponentType === MessageComponentType.Audio) {
                return i18nc("@action:inmenu", "Open Audio");
            } else if (root.messageComponentType === MessageComponentType.Video) {
                return i18nc("@action:inmenu", "Open Video");
            } else {
                return i18nc("@action:inmenu", "Open File");
            }
        }
        icon.name: "document-open"
        onTriggered: {
            currentRoom.openEventMediaExternally(root.eventId);
        }
    }

    Kirigami.Action {
        visible: root.hasFileContent
        text: {
            if (root.messageComponentType === MessageComponentType.Image) {
                return i18nc("@action:inmenu", "Save Image…");
            } else if (root.messageComponentType === MessageComponentType.Audio) {
                return i18nc("@action:inmenu", "Save Audio…");
            } else if (root.messageComponentType === MessageComponentType.Video) {
                return i18nc("@action:inmenu", "Save Video…");
            } else {
                return i18nc("@action:inmenu", "Save File…");
            }
        }
        icon.name: "document-save"
        onTriggered: {
            var dialog = saveAsDialog.createObject(QQC2.Overlay.overlay);
            dialog.selectedFile = currentRoom.fileNameToDownload(eventId);
            dialog.open();
        }
    }

    Kirigami.Action {
        visible: root.hasFileContent
        text: {
            if (root.messageComponentType === MessageComponentType.Image) {
                return i18nc("@action:inmenu", "Copy Image");
            } else if (root.messageComponentType === MessageComponentType.Audio) {
                return i18nc("@action:inmenu", "Copy Audio");
            } else if (root.messageComponentType === MessageComponentType.Video) {
                return i18nc("@action:inmenu", "Copy Video");
            } else {
                return i18nc("@action:inmenu", "Copy File");
            }
        }
        icon.name: "edit-copy"
        onTriggered: {
            currentRoom.copyEventMedia(root.eventId);
        }
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Copy Link Address")
        icon.name: "edit-copy"
        visible: root.hoveredLink.length > 0
        onTriggered: Clipboard.saveText(root.hoveredLink)
    }

    Kirigami.Action {
        visible: !root.hasFileContent
        text: i18nc("@action:inmenu", "Copy Text")
        icon.name: "edit-copy"
        onTriggered: Clipboard.saveText(root.selectedText.length > 0 ? root.selectedText : root.plainText)
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Copy Message Link")
        icon.name: "link-symbolic"
        onTriggered: {
            Clipboard.saveText("https://matrix.to/#/" + currentRoom.id + "/" + root.eventId);
        }
    }

    Kirigami.Action {
        text: i18nc("@action:button 'Report' as in 'Report this event to the administrators'", "Report…")
        icon.name: "dialog-warning-symbolic"
        visible: !author.isLocalMember
        onTriggered: {
            let dialog = (root.Kirigami.PageStack.pageStack as Kirigami.PageRow).pushDialogLayer(Qt.createComponent('org.kde.neochat', 'ReasonDialog'), {
                title: i18nc("@title:dialog", "Report Message"),
                placeholder: i18nc("@info:placeholder", "Reason for reporting this message"),
                icon: "dialog-warning-symbolic",
                actionText: i18nc("@action:button 'Report' as in 'Report this event to the administrators'", "Report")
            }, {
                title: i18nc("@title", "Report Message"),
                width: Kirigami.Units.gridUnit * 25
            });
            dialog.accepted.connect(reason => {
                currentRoom.reportEvent(root.eventId, reason);
            });
        }
    }

    Kirigami.Action {
        id: webShortcutModelAction

        text: i18nc("@action:inmenu", "Search for '%1'", webShortcutModel.trunkatedSearchText)
        icon.name: "search-symbolic"
        visible: !root.hasFileContent && webShortcutModel.enabled

        readonly property Instantiator instantiator: Instantiator {
            model: WebShortcutModel {
                id: webShortcutModel
                selectedText: root.selectedText.length > 0 ? root.selectedText : root.plainText
                onOpenUrl: url => RoomManager.resolveResource(url.toString())
            }
            delegate: Kirigami.Action {
                text: model.display
                icon.name: model.decoration
                onTriggered: webShortcutModel.trigger(model.edit)
            }
            onObjectAdded: (index, object) => webShortcutModelAction.children.push(object)
        }
    }

    Kirigami.Action {
        text: i18nc("@action:inmenu", "Configure Web Shortcuts…")
        icon.name: "configure"
        visible: !Controller.isFlatpak && webShortcutModel.enabled
        onTriggered: webShortcutModel.configureWebShortcuts()
    }

    Kirigami.Action {
        visible: !root.hasFileContent
        text: i18nc("@action:inmenu", "Read Text Aloud")
        icon.name: "audio-speakers-symbolic"
        onTriggered: {
            TextToSpeechHelper.speak(i18nc("@info text-to-speech %1 is author %2 is message text", "%1 said %2", root.author.displayName, root.plainText))
        }
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

    Kirigami.Action {
        readonly property bool pinned: currentRoom.isEventPinned(root.eventId)

        visible: currentRoom.canSendState("m.room.pinned_events") && root.messageComponentType !== MessageComponentType.Other
        text: pinned ? i18nc("@action:button 'Unpin' as in 'Unpin this message'", "Unpin") : i18nc("@action:button 'Pin' as in 'Pin the message in the room'", "Pin")
        icon.name: pinned ? "window-unpin-symbolic" : "pin-symbolic"
        onTriggered: pinned ? currentRoom.unpinEvent(root.eventId) : currentRoom.pinEvent(root.eventId)
    }

    Kirigami.Action {
        separator: true
        visible: viewSourceAction.visible
    }

    Kirigami.Action {
        id: viewSourceAction
        visible: NeoChatConfig.developerTools
        text: i18nc("@action:inmenu", "View Source")
        icon.name: "code-context"
        onTriggered: RoomManager.viewEventSource(root.eventId)
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

    headerContentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        KirigamiComponents.Avatar {
            source: root.author.avatarUrl

            Layout.preferredWidth: Kirigami.Units.gridUnit * 2
            Layout.preferredHeight: Kirigami.Units.gridUnit * 2
            Layout.alignment: Qt.AlignTop
        }

        ColumnLayout {
            spacing: 0

            Layout.fillWidth: true

            Kirigami.Heading {
                level: 4
                text: root.author.htmlSafeDisplayName
                wrapMode: Text.WordWrap
                Layout.fillWidth: true
            }

            QQC2.Label {
                text: root.plainText
                textFormat: Text.PlainText
                elide: Text.ElideRight
                onLinkActivated: RoomManager.resolveResource(link, "join")
                Layout.fillWidth: true
            }
        }
    }
}
