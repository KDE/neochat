// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat
import org.kde.neochat.chatbar

/**
 * @brief A component to show a chat bar in a message bubble.
 */
QQC2.TextArea {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The ChatBarCache to use.
     */
    required property ChatBarCache chatBarCache
    onChatBarCacheChanged: documentHandler.chatBarCache = chatBarCache

    /**
     * @brief The ActionsHandler object to use.
     *
     * This is expected to have the correct room set otherwise messages will be sent
     * to the wrong room.
     */
    required property ActionsHandler actionsHandler

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.fillWidth: true
    Layout.preferredWidth: textMetrics.advanceWidth + rightPadding + Kirigami.Units.smallSpacing + Kirigami.Units.gridUnit
    Layout.maximumWidth: root.maxContentWidth
    Layout.minimumHeight: chatButtons.height + topPadding + bottomPadding

    Component.onCompleted: _private.updateText()

    topPadding: Kirigami.Units.smallSpacing
    bottomPadding: Kirigami.Units.smallSpacing
    rightPadding: chatButtons.width + chatButtons.anchors.rightMargin * 2

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
            root.insert(cursorPosition, "\n");
        } else {
            _private.post();
        }
    }
    Keys.onReturnPressed: {
        if (completionMenu.visible) {
            completionMenu.complete();
        } else if (event.modifiers & Qt.ShiftModifier) {
            root.insert(cursorPosition, "\n");
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

    /**
    * This is anchored like this so that control expands properly as the
    * text grows in length.
    */
    RowLayout {
        id: chatButtons
        anchors.verticalCenter: root.verticalCenter
        anchors.right: root.right
        anchors.rightMargin: Kirigami.Units.smallSpacing
        spacing: 0
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

    CompletionMenu {
        id: completionMenu
        height: implicitHeight
        y: -height - 5
        z: 10
        connection: root.room.connection
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
        document: root.textDocument
        cursorPosition: root.cursorPosition
        selectionStart: root.selectionStart
        selectionEnd: root.selectionEnd
        room: root.room // We don't care about saving for edits so this is OK.
        mentionColor: Kirigami.Theme.linkColor
        errorColor: Kirigami.Theme.negativeTextColor
    }

    TextMetrics {
        id: textMetrics
        text: root.text
    }

    QtObject {
        id: _private

        function updateText() {
            // This could possibly be undefined due to some esoteric QtQuick issue. Referencing it somewhere in JS is enough.
            documentHandler.document;
            if (chatBarCache?.isEditing && chatBarCache.relationMessage.length > 0) {
                root.text = chatBarCache.relationMessage;
                root.chatBarCache.updateMentions(root.textDocument, documentHandler);
                root.forceActiveFocus();
                root.cursorPosition = root.text.length;
            }
        }

        function post() {
            root.actionsHandler.handleMessageEvent(root.chatBarCache);
            root.clear();
            root.chatBarCache.clearRelations();
        }
    }
}
