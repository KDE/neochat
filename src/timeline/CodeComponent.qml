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
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief The attributes of the component.
     */
    required property var componentAttributes

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    /**
     * @brief The user selected text has changed.
     */
    signal selectedTextChanged(string selectedText)

    /**
     * @brief Request a context menu be show for the message.
     */
    signal showMessageMenu

    Layout.fillWidth: true
    Layout.fillHeight: true
    Layout.maximumWidth: root.maxContentWidth
    Layout.maximumHeight: Kirigami.Units.gridUnit * 20

    topPadding: 0
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0

    contentItem: QQC2.ScrollView {
        id: codeScrollView
        contentWidth: root.maxContentWidth

        // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        QQC2.TextArea {
            id: codeText
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing
            leftPadding: lineNumberColumn.width + lineNumberColumn.anchors.leftMargin + Kirigami.Units.smallSpacing * 2

            text: root.display
            readOnly: true
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
                acceptedButtons: Qt.LeftButton
                onLongPressed: root.showMessageMenu()
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

    QQC2.Button {
        anchors {
            top: parent.top
            topMargin: Kirigami.Units.smallSpacing
            right: parent.right
            rightMargin: (codeScrollView.QQC2.ScrollBar.vertical.visible ? codeScrollView.QQC2.ScrollBar.vertical.width : 0) + Kirigami.Units.smallSpacing
        }
        visible: root.hovered
        icon.name: "edit-copy"
        text: i18n("Copy to clipboard")
        display: QQC2.AbstractButton.IconOnly

        onClicked: Clipboard.saveText(root.display);

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing
        border {
            width: root.hovered ? 1 : 0
            color: Kirigami.Theme.highlightColor
        }

    }
}
