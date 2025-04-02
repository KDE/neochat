// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtCore
import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.chatbar

/**
 * @brief A component to show a chat bar in a message bubble.
 */
QQC2.Control {
    id: root

    /**
     * @brief The ChatBarCache to use.
     */
    required property ChatBarCache chatBarCache
    onChatBarCacheChanged: documentHandler.chatBarCache = chatBarCache

    readonly property bool isBusy: root.Message.room && root.Message.room.hasFileUploading

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    contentItem: ColumnLayout {
        Loader {
            id: paneLoader

            Layout.fillWidth: true
            Layout.margins: Kirigami.Units.largeSpacing
            Layout.preferredHeight: active ? item.implicitHeight : 0

            active: visible
            visible: root.chatBarCache.replyId.length > 0 || root.chatBarCache.attachmentPath.length > 0
            sourceComponent: root.chatBarCache.replyId.length > 0 ? replyPane : attachmentPane
        }
        RowLayout {
            Layout.fillWidth: true
            spacing: 0

            QQC2.ScrollView {
                id: chatBarScrollView
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Kirigami.Units.smallSpacing
                Layout.leftMargin: Kirigami.Units.largeSpacing
                Layout.rightMargin: Kirigami.Units.largeSpacing

                Layout.fillWidth: true
                Layout.maximumHeight: Kirigami.Units.gridUnit * 8

                QQC2.TextArea {
                    id: textArea
                    Component.onCompleted: _private.updateText()

                    Layout.fillWidth: true

                    color: Kirigami.Theme.textColor
                    verticalAlignment: TextEdit.AlignVCenter
                    wrapMode: TextEdit.Wrap

                    onTextChanged: {
                        root.chatBarCache.text = text;
                    }

                    Keys.onEnterPressed: {
                        if (completionMenu.visible) {
                            completionMenu.complete();
                        } else if (event.modifiers & Qt.ShiftModifier) {
                            textArea.insert(cursorPosition, "\n");
                        } else {
                            _private.post();
                        }
                    }
                    Keys.onReturnPressed: {
                        if (completionMenu.visible) {
                            completionMenu.complete();
                        } else if (event.modifiers & Qt.ShiftModifier) {
                            textArea.insert(cursorPosition, "\n");
                        } else {
                            _private.post();
                        }
                    }
                    Keys.onTabPressed: {
                        if (completionMenu.visible) {
                            completionMenu.complete();
                        }
                    }
                    Keys.onPressed: event => {
                        if (event.key === Qt.Key_Up && completionMenu.visible) {
                            completionMenu.decrementIndex();
                        } else if (event.key === Qt.Key_Down && completionMenu.visible) {
                            completionMenu.incrementIndex();
                        }
                    }

                    CompletionMenu {
                        id: completionMenu
                        height: implicitHeight
                        y: -height - 5
                        z: 10
                        connection: root.Message.room.connection
                        chatDocumentHandler: documentHandler
                        Behavior on height {
                            NumberAnimation {
                                property: "height"
                                duration: Kirigami.Units.shortDuration
                                easing.type: Easing.OutCubic
                            }
                        }
                    }

                    // opt-out of whatever spell checker a styled TextArea might come with
                    Kirigami.SpellCheck.enabled: false

                    ChatDocumentHandler {
                        id: documentHandler
                        document: textArea.textDocument
                        cursorPosition: textArea.cursorPosition
                        selectionStart: textArea.selectionStart
                        selectionEnd: textArea.selectionEnd
                        room: root.Message.room // We don't care about saving for edits so this is OK.
                        mentionColor: Kirigami.Theme.linkColor
                        errorColor: Kirigami.Theme.negativeTextColor
                    }

                    TextMetrics {
                        id: textMetrics
                        text: textArea.text
                    }

                    Component {
                        id: openFileDialog

                        OpenFileDialog {
                            parentWindow: Window.window
                            currentFolder: StandardPaths.standardLocations(StandardPaths.HomeLocation)[0]
                        }
                    }

                    Component {
                        id: attachDialog

                        AttachDialog {
                            anchors.centerIn: parent
                        }
                    }

                    background: null
                }
            }
            PieProgressBar {
                visible: root.isBusy
                progress: root.Message.room.fileUploadingProgress
            }
            QQC2.ToolButton {
                visible: !root.isBusy
                display: QQC2.AbstractButton.IconOnly
                action: Kirigami.Action {
                    text: i18nc("@action:button", "Attach an image or file")
                    icon.name: "mail-attachment"
                    onTriggered: {
                        let dialog = (Clipboard.hasImage ? attachDialog : openFileDialog).createObject(QQC2.Overlay.overlay);
                        dialog.chosen.connect(path => root.chatBarCache.attachmentPath = path);
                        dialog.open();
                    }
                }
                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
            }
            QQC2.ToolButton {
                display: QQC2.AbstractButton.IconOnly
                action: Kirigami.Action {
                    text: root.chatBarCache.isEditing ? i18nc("@action:button", "Confirm edit") : i18nc("@action:button", "Post message in thread")
                    icon.name: "document-send"
                    onTriggered: {
                        _private.post();
                    }
                }
                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
            }
            QQC2.ToolButton {
                display: QQC2.AbstractButton.IconOnly
                action: Kirigami.Action {
                    text: i18nc("@action:button", "Cancel")
                    icon.name: "dialog-close"
                    onTriggered: {
                        root.chatBarCache.clearRelations();
                    }
                    shortcut: "Escape"
                }
                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
            }
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.cornerRadius
        border {
            width: 1
            color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
        }
    }

    Component {
        id: replyPane
        Item {
            implicitWidth: replyComponent.implicitWidth
            implicitHeight: replyComponent.implicitHeight
            ReplyComponent {
                id: replyComponent
                replyEventId: root.chatBarCache.replyId
                replyAuthor: root.chatBarCache.relationAuthor
                replyContentModel: root.chatBarCache.relationEventContentModel
                Message.maxContentWidth: paneLoader.item.width
            }
            QQC2.Button {
                id: cancelButton

                anchors.top: parent.top
                anchors.right: parent.right

                display: QQC2.AbstractButton.IconOnly
                text: i18nc("@action:button", "Cancel reply")
                icon.name: "dialog-close"
                onClicked: {
                    root.chatBarCache.replyId = "";
                    root.chatBarCache.attachmentPath = "";
                }
                QQC2.ToolTip.text: text
                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }
    }
    Component {
        id: attachmentPane
        AttachmentPane {
            attachmentPath: root.chatBarCache.attachmentPath

            onAttachmentCancelled: {
                root.chatBarCache.attachmentPath = "";
                root.forceActiveFocus();
            }
        }
    }

    QtObject {
        id: _private

        function updateText() {
            // This could possibly be undefined due to some esoteric QtQuick issue. Referencing it somewhere in JS is enough.
            documentHandler.document;
            if (chatBarCache?.isEditing && chatBarCache.relationMessage.length > 0) {
                textArea.text = chatBarCache.relationMessage;
                root.chatBarCache.updateMentions(textArea.textDocument, documentHandler);
                textArea.forceActiveFocus();
                textArea.cursorPosition = textArea.text.length;
            }
        }

        function post() {
            root.chatBarCache.postMessage();
            textArea.clear();
            root.chatBarCache.clearRelations();
        }
    }
}
