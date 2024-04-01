// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigamiaddons.labs.components as Components
import org.kde.kirigami as Kirigami
import org.kde.syntaxhighlighting

import org.kde.neochat

Components.AbstractMaximizeComponent {
    id: root

    /**
     * @brief The message author.
     *
     * This should consist of the following:
     *  - id - The matrix ID of the author.
     *  - isLocalUser - Whether the author is the local user.
     *  - avatarSource - The mxc URL for the author's avatar in the current room.
     *  - avatarMediaId - The media ID of the author's avatar.
     *  - avatarUrl - The mxc URL for the author's avatar.
     *  - displayName - The display name of the author.
     *  - display - The name of the author.
     *  - color - The color for the author.
     *  - object - The Quotient::User object for the author.
     *
     * @sa Quotient::User
     */
    property var author

    /**
     * @brief The timestamp of the message.
     */
    property var time

    /**
     * @brief The code text to show.
     */
    property string codeText

    /**
     * @brief The code language, if any.
     */
    property string language

    actions: [
        Kirigami.Action {
            text: i18nc("@action", "Copy to clipboard")
            icon.name: "edit-copy"
            onTriggered: Clipboard.saveText(root.codeText)
        }
    ]

    leading: RowLayout {
        Components.Avatar {
            id: userAvatar
            implicitWidth: Kirigami.Units.iconSizes.medium
            implicitHeight: Kirigami.Units.iconSizes.medium

            name: root.author.name ?? root.author.displayName
            source: root.author.avatarSource
            color: root.author.color
        }
        ColumnLayout {
            spacing: 0
            QQC2.Label {
                id: userLabel

                text: root.author.name ?? root.author.displayName
                color: root.author.color
                font.weight: Font.Bold
                elide: Text.ElideRight
            }
            QQC2.Label {
                id: dateTimeLabel
                text: root.time.toLocaleString(Qt.locale(), Locale.ShortFormat)
                color: Kirigami.Theme.disabledTextColor
                elide: Text.ElideRight
            }
        }
    }

    content: QQC2.ScrollView {
        id: codeScrollView
        contentWidth: root.width

        // HACK: Hide unnecessary horizontal scrollbar (https://bugreports.qt.io/browse/QTBUG-83890)
        QQC2.ScrollBar.horizontal.policy: QQC2.ScrollBar.AlwaysOff

        QQC2.TextArea {
            id: codeText
            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing
            leftPadding: lineNumberColumn.width + lineNumberColumn.anchors.leftMargin + Kirigami.Units.smallSpacing * 2

            text: root.codeText
            readOnly: true
            textFormat: TextEdit.PlainText
            wrapMode: TextEdit.Wrap
            color: Kirigami.Theme.textColor

            font.family: "monospace"

            Kirigami.SpellCheck.enabled: false

            onWidthChanged: lineModel.resetModel()
            onHeightChanged: lineModel.resetModel()

            SyntaxHighlighter {
                property string definitionName: Repository.definitionForName(root.language).name
                textEdit: definitionName == "None" ? null : codeText
                definition: definitionName
            }
            ColumnLayout {
                id: lineNumberColumn
                anchors {
                    top: codeText.top
                    topMargin: codeText.topPadding + 1
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
            Kirigami.Separator {
                anchors {
                    top: parent.top
                    bottom: parent.bottom
                    left: parent.left
                    leftMargin: lineNumberColumn.width + lineNumberColumn.anchors.leftMargin + Kirigami.Units.smallSpacing
                }
            }
            TapHandler {
                acceptedButtons: Qt.LeftButton
                onTapped: root.close()
            }

            background: null
        }

        background: Rectangle {
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            Kirigami.Theme.inherit: false
            color: Kirigami.Theme.backgroundColor
        }
    }
}
