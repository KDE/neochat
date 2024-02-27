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

    topPadding: 0
    bottomPadding: 0

    contentItem: RowLayout {
        spacing: Kirigami.Units.smallSpacing

        ColumnLayout {
            id: lineNumberColumn
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
        Kirigami.Separator {
            Layout.fillHeight: true
        }
        TextEdit {
            id: codeText
            Layout.fillWidth: true
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing

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

            TapHandler {
                acceptedButtons: Qt.LeftButton
                onLongPressed: root.showMessageMenu()
            }
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
        radius: Kirigami.Units.smallSpacing
    }
}
