// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.neochat 1.0

QQC2.TextArea {
    id: root

    property string messageId

    Layout.fillWidth: true
    Layout.minimumHeight: editButtons.height + topPadding + bottomPadding
    Layout.preferredWidth: editTextMetrics.advanceWidth + rightPadding + Kirigami.Units.smallSpacing + Kirigami.Units.gridUnit
    rightPadding: editButtons.width + editButtons.anchors.rightMargin * 2

    color: Kirigami.Theme.textColor
    verticalAlignment: TextEdit.AlignVCenter
    wrapMode: Text.Wrap

    onVisibleChanged: {
        if (visible) {
            forceActiveFocus();
            root.cursorPosition = root.length;
        }
    }
    onTextChanged: {
        currentRoom.editText = text
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
    Keys.onPressed: {
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
                    currentRoom.chatBoxEditId = "";
                }
                shortcut: "Escape"
            }
            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
        }
    }

    Connections {
        target: currentRoom
        function onChatBoxEditIdChanged() {
            if (currentRoom.chatBoxEditId == messageId && currentRoom.chatBoxEditMessage.length > 0) {
                root.text = currentRoom.chatBoxEditMessage
            }
        }
    }

    CompletionMenu {
        id: completionMenu
        height: implicitHeight
        y: -height - 5
        z: 10
        chatDocumentHandler: documentHandler
        Behavior on height {
            NumberAnimation {
                property: "height"
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    ChatDocumentHandler {
        id: documentHandler
        isEdit: true
        document: root.textDocument
        cursorPosition: root.cursorPosition
        selectionStart: root.selectionStart
        selectionEnd: root.selectionEnd
        room: currentRoom // We don't care about saving for edits so this is OK.
    }

    TextMetrics {
        id: editTextMetrics
        text: root.text
    }

    function postEdit() {
        actionsHandler.handleEdit();
        root.clear();
        currentRoom.chatBoxEditId = "";
    }
}


