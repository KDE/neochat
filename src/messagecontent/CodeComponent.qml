// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.syntaxhighlighting

import org.kde.neochat

QQC2.Control {
    id: root

    /**
     * @brief The index of the delegate in the model.
     */
    required property int index

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
    required property NeochatRoomMember author

    /**
     * @brief The timestamp of the message.
     */
    required property var time

    /**
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief Whether the component should be editable.
     */
    required property bool editable

    /**
     * @brief The attributes of the component.
     */
    required property var componentAttributes
    readonly property ChatDocumentHandler chatDocumentHandler: componentAttributes?.chatDocumentHandler ?? null
    onChatDocumentHandlerChanged: if (chatDocumentHandler) {
        chatDocumentHandler.type = ChatBarType.Room;
        chatDocumentHandler.room = root.Message.room;
        chatDocumentHandler.textItem = codeText;
    }

    /**
     * @brief Whether the component is currently focussed.
     */
    required property bool currentFocus
    onCurrentFocusChanged: if (currentFocus && !codeText.focus) {
        codeText.forceActiveFocus();
    }

    /**
     * @brief The user selected text has changed.
     */
    signal selectedTextChanged(string selectedText)

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.maximumWidth: Message.maxContentWidth
    Layout.maximumHeight: Kirigami.Units.gridUnit * 20

    width: ListView.view?.width ?? -1

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    contentItem: QQC2.ScrollView {
        id: codeScrollView
        contentWidth: root.Message.maxContentWidth

        // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        QQC2.TextArea {
            id: codeText

            Keys.onUpPressed: (event) => {
                event.accepted = false;
                if (root.chatDocumentHandler.atFirstLine) {
                    Message.contentModel.focusRow = root.index - 1
                }
            }
            Keys.onDownPressed: (event) => {
                event.accepted = false;
                if (root.chatDocumentHandler.atLastLine) {
                    Message.contentModel.focusRow = root.index + 1
                }
            }

            Keys.onDeletePressed: (event) => {
                event.accepted = true;
                root.chatDocumentHandler.deleteChar();
            }

            Keys.onPressed: (event) => {
                if (event.key == Qt.Key_Backspace && cursorPosition == 0) {
                    event.accepted = true;
                    root.chatDocumentHandler.backspace();
                    return;
                }
                event.accepted = false;
            }

            onFocusChanged: if (focus && !root.currentFocus) {
                Message.contentModel.setFocusRow(root.index, true)
            }

            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing
            leftPadding: lineNumberColumn.width + lineNumberColumn.anchors.leftMargin + Kirigami.Units.smallSpacing * 2

            text: root.editable ? "" : root.display
            readOnly: !root.editable
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.Wrap
            color: Kirigami.Theme.textColor

            font.family: "monospace"

            Kirigami.SpellCheck.enabled: false

            onWidthChanged: lineModel.resetModel()
            onHeightChanged: lineModel.resetModel()

            onSelectedTextChanged: root.selectedTextChanged(selectedText)

            SyntaxHighlighter {
                property string definitionName: Repository.definitionForName(root.componentAttributes.class).name
                textEdit: definitionName == "None" ? null : codeText
                definition: definitionName
            }
            ColumnLayout {
                id: lineNumberColumn
                anchors {
                    top: codeText.top
                    topMargin: codeText.topPadding
                    left: codeText.left
                    leftMargin: Kirigami.Units.smallSpacing
                }
                spacing: 0
                Repeater {
                    id: repeater
                    model: LineModel {
                        id: lineModel
                        document: codeText.textDocument
                    }
                    delegate: QQC2.Label {
                        id: label
                        required property int index
                        required property int docLineHeight
                        Layout.fillWidth: true
                        Layout.preferredHeight: docLineHeight
                        horizontalAlignment: Text.AlignRight
                        text: index + 1
                        color: Kirigami.Theme.disabledTextColor

                        font.family: "monospace"
                    }
                }
            }

            TapHandler {
                enabled: root.time.toString() !== "Invalid Date"
                acceptedButtons: Qt.LeftButton
                acceptedDevices: PointerDevice.Mouse | PointerDevice.TouchPad | PointerDevice.Stylus
                onTapped: RoomManager.maximizeCode(root.author, root.time, root.display, root.componentAttributes.class)
            }

            TapHandler {
                acceptedDevices: PointerDevice.TouchScreen
                onLongPressed: RoomManager.viewEventMenu(root.eventId, root.Message.room, root.Message.selectedText, root.Message.hoveredLink);
            }

            background: null
        }
    }

    Kirigami.Separator {
        anchors {
            top: root.top
            bottom: root.bottom
            left: root.left
            leftMargin: lineNumberColumn.width + lineNumberColumn.anchors.leftMargin + Kirigami.Units.smallSpacing
        }
    }

    RowLayout {
        anchors {
            top: parent.top
            topMargin: Kirigami.Units.smallSpacing
            right: parent.right
            rightMargin: (codeScrollView.QQC2.ScrollBar.vertical.visible ? codeScrollView.QQC2.ScrollBar.vertical.width : 0) + Kirigami.Units.smallSpacing
        }
        visible: root.hovered && !root.editable
        spacing: Kirigami.Units.mediumSpacing

        QQC2.Button {
            icon.name: "edit-copy"
            text: i18n("Copy to clipboard")
            display: QQC2.AbstractButton.IconOnly

            onClicked: Clipboard.saveText(root.display);

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.Button {
            visible: root.time.toString() !== "Invalid Date"
            icon.name: "view-fullscreen"
            text: i18nc("@action:button", "Maximize")
            display: QQC2.AbstractButton.IconOnly

            onClicked: RoomManager.maximizeCode(root.author, root.time, root.display, root.componentAttributes.class);

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        Kirigami.Theme.colorSet: Kirigami.Theme.View
        Kirigami.Theme.inherit: false
        radius: Kirigami.Units.cornerRadius
        border {
            width: 1
            color: Kirigami.Theme.highlightColor
        }

    }
}
