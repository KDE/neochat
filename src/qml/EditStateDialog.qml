// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.syntaxhighlighting

import org.kde.neochat

Kirigami.Page {
    id: root

    required property string sourceText
    property bool allowEdit: false

    property NeoChatRoom room
    property string type
    property string stateKey

    topPadding: 0
    leftPadding: 0
    rightPadding: 0
    bottomPadding: 0

    title: i18nc("@title As in 'edit the state of this room'", "Edit State")

    actions: [
        Kirigami.Action {
            text: i18nc("@action", "Revert changes")
            icon.name: "document-revert"
            onTriggered: sourceTextArea.text = root.sourceText
            enabled: sourceTextArea.text !== root.sourceText
        },
        Kirigami.Action {
            text: i18nc("@action As in 'Apply the changes'", "Apply")
            icon.name: "document-edit"
            onTriggered: {
                root.room.setRoomState(root.type, root.stateKey, sourceTextArea.text);
                root.closeDialog();
            }
            enabled: QmlUtils.isValidJson(sourceTextArea.text)
        }
    ]

    QQC2.ScrollView {
        id: scrollView
        anchors.fill: parent
        contentWidth: availableWidth

        QQC2.TextArea {
            id: sourceTextArea
            Layout.fillWidth: true

            leftPadding: lineNumberColumn.width + lineNumberColumn.anchors.leftMargin + Kirigami.Units.smallSpacing * 2

            text: root.sourceText
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.Wrap

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
