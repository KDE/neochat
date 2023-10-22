// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.TextArea {
    id: root

    required property NeoChatRoom room
    onRoomChanged: {
        _private.chatBarCache = room.editCache
        _private.chatBarCache.relationIdChanged.connect(_private.updateEditText)
    }

    /**
     * @brief The ActionsHandler object to use.
     *
     * This is expected to have the correct room set otherwise messages will be sent
     * to the wrong room.
     */
    required property ActionsHandler actionsHandler

    property string messageId

    property var minimumHeight: editButtons.height + topPadding + bottomPadding
    property var preferredWidth: editTextMetrics.advanceWidth + rightPadding + Kirigami.Units.smallSpacing + Kirigami.Units.gridUnit
    rightPadding: editButtons.width + editButtons.anchors.rightMargin * 2

    color: Kirigami.Theme.textColor
    verticalAlignment: TextEdit.AlignVCenter
    wrapMode: Text.Wrap

    onTextChanged: {
        _private.chatBarCache.text = text
    }

    Keys.onEnterPressed: {
        if (completionMenu.visible) {
            completionMenu.complete()
        } else if (event.modifiers & Qt.ShiftModifier) {
            root.insert(cursorPosition, "\n")
        } else {
            root.postEdit();
        }
    }
    Keys.onReturnPressed: {
        if (completionMenu.visible) {
            completionMenu.complete()
        } else if (event.modifiers & Qt.ShiftModifier) {
            root.insert(cursorPosition, "\n")
        } else {
            root.postEdit();
        }
    }
    Keys.onTabPressed: {
        if (completionMenu.visible) {
            completionMenu.complete()
        }
    }
    Keys.onPressed: event => {
        if (event.key === Qt.Key_Up && completionMenu.visible) {
            completionMenu.decrementIndex()
        } else if (event.key === Qt.Key_Down && completionMenu.visible) {
            completionMenu.incrementIndex()
        }
    }

    /**
    * This is anchored like this so that control expands properly as the edited
    * text grows in length.
    */
    RowLayout {
        id: editButtons
        anchors.verticalCenter: root.verticalCenter
        anchors.right: root.right
        anchors.rightMargin: Kirigami.Units.smallSpacing
        spacing: 0
        QQC2.ToolButton {
            display: QQC2.AbstractButton.IconOnly
            action: Kirigami.Action {
                text: i18nc("@action:button", "Confirm edit")
                icon.name: "checkmark"
                onTriggered: {
                    root.postEdit();
                }
            }
            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
        }
        QQC2.ToolButton {
            display: QQC2.AbstractButton.IconOnly
            action: Kirigami.Action {
                text: i18nc("@action:button", "Cancel edit")
                icon.name: "dialog-close"
                onTriggered: {
                    _private.chatBarCache.editId = "";
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
        isEdit: true
        document: root.textDocument
        cursorPosition: root.cursorPosition
        selectionStart: root.selectionStart
        selectionEnd: root.selectionEnd
        room: root.room // We don't care about saving for edits so this is OK.
        mentionColor: Kirigami.Theme.linkColor
        errorColor: Kirigami.Theme.negativeTextColor
    }

    TextMetrics {
        id: editTextMetrics
        text: root.text
    }

    function postEdit() {
        root.actionsHandler.handleMessageEvent(_private.chatBarCache);
        root.clear();
        _private.chatBarCache.editId = "";
    }

    QtObject {
        id: _private
        property ChatBarCache chatBarCache
        onChatBarCacheChanged: documentHandler.chatBarCache = chatBarCache

        function updateEditText() {
            if (chatBarCache.isEditing && chatBarCache.relationMessage.length > 0) {
                root.text = chatBarCache.relationMessage
                root.forceActiveFocus();
                root.cursorPosition = root.length;
            }
        }
    }
}


