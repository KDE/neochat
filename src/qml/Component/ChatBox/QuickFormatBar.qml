// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

QQC2.Popup {
    id: root

    property var selectionStart
    property var selectionEnd

    signal formattingSelected(var format, int selectionStart, int selectionEnd)

    padding: 1

    contentItem: Flow {
        QQC2.ToolButton {
            icon.name: "format-text-bold"
            text: i18n("Bold")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "**",
                    end: "**",
                    extra: "",
                }
                formattingSelected(format, selectionStart, selectionEnd)
                root.close()
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "format-text-italic"
            text: i18n("Italic")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "*",
                    end: "*",
                    extra: "",
                }
                formattingSelected(format, selectionStart, selectionEnd)
                root.close()
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "format-text-strikethrough"
            text: i18n("Strikethrough")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "<del>",
                    end: "</del>",
                    extra: "",
                }
                formattingSelected(format, selectionStart, selectionEnd)
                root.close()
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "format-text-code"
            text: i18n("Code block")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "`",
                    end: "`",
                    extra: "",
                }
                formattingSelected(format, selectionStart, selectionEnd)
                root.close()
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "format-text-blockquote"
            text: i18n("Quote")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: selectionStart == 0 ? ">" : "\n>",
                    end: "\n\n",
                    extra: "",
                }
                formattingSelected(format, selectionStart, selectionEnd)
                root.close()
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "link"
            text: i18n("Insert link")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "[",
                    end: "](",
                    extra: ")",
                }
                formattingSelected(format, selectionStart, selectionEnd)
                root.close()
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }
}
