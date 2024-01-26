// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.syntaxhighlighting

import org.kde.neochat

Kirigami.Page {
    id: root
    property string sourceText

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    title: i18n("Event Source")

    QQC2.ScrollView {
        id: scrollView
        anchors.fill: parent
        contentWidth: availableWidth

        // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        QQC2.TextArea {
            id: sourceTextArea
            Layout.fillWidth: true

            leftPadding: lineNumberColumn.width + lineNumberColumn.anchors.leftMargin + Kirigami.Units.smallSpacing * 2

            text: root.sourceText
            readOnly: true
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.Wrap

            // opt-out of whatever spell checker a styled TextArea might come with
            Kirigami.SpellCheck.enabled: false

            onWidthChanged: lineModel.resetModel()
            onHeightChanged: lineModel.resetModel()

            SyntaxHighlighter {
                textEdit: sourceTextArea
                definition: "JSON"
                repository: Repository
            }

            ColumnLayout {
                id: lineNumberColumn

                anchors {
                    top: sourceTextArea.top
                    topMargin: sourceTextArea.topPadding
                    left: sourceTextArea.left
                    leftMargin: Kirigami.Units.smallSpacing
                }
                spacing: 0
                Repeater {
                    id: repeater
                    model: LineModel {
                        id: lineModel
                        document: sourceTextArea.textDocument
                    }
                    delegate: QQC2.Label {
                        id: label
                        required property int index
                        required property int docLineHeight
                        Layout.fillWidth: true
                        Layout.preferredHeight: docLineHeight
                        topPadding: 1
                        horizontalAlignment: Text.AlignRight
                        text: index + 1
                        color: Kirigami.Theme.disabledTextColor
                    }
                }
            }

            background: Rectangle {
                Kirigami.Theme.colorSet: Kirigami.Theme.View
                Kirigami.Theme.inherit: false
                color: Kirigami.Theme.backgroundColor
            }
        }
    }

    Kirigami.Separator {
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: parent.left
            leftMargin: lineNumberColumn.width + lineNumberColumn.anchors.leftMargin + Kirigami.Units.smallSpacing
        }
    }
}

