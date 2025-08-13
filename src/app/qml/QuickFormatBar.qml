// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2

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
            text: i18nc("@action:button", "Bold")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "**",
                    end: "**",
                    extra: ""
                };
                root.formattingSelected(format, root.selectionStart, root.selectionEnd);
                root.close();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "format-text-italic"
            text: i18nc("@action:button", "Italic")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "*",
                    end: "*",
                    extra: ""
                };
                root.formattingSelected(format, root.selectionStart, root.selectionEnd);
                root.close();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "format-text-strikethrough"
            text: i18nc("@action:button", "Strikethrough")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "~~",
                    end: "~~",
                    extra: ""
                };
                root.formattingSelected(format, root.selectionStart, root.selectionEnd);
                root.close();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "view-hidden-symbolic"
            text: i18nc("@action:button", "Spoiler")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "||",
                    end: "||",
                    extra: ""
                };
                root.formattingSelected(format, root.selectionStart, root.selectionEnd);
                root.close();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "format-text-code"
            text: i18nc("@action:button", "Code block")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "`",
                    end: "`",
                    extra: ""
                };
                root.formattingSelected(format, root.selectionStart, root.selectionEnd);
                root.close();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "format-text-blockquote"
            text: i18nc("@action:button", "Quote")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: root.selectionStart == 0 ? ">" : "\n>",
                    end: "\n\n",
                    extra: ""
                };
                root.formattingSelected(format, root.selectionStart, root.selectionEnd);
                root.close();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
        QQC2.ToolButton {
            icon.name: "link"
            text: i18nc("@action:button", "Insert link")
            display: QQC2.AbstractButton.IconOnly

            onClicked: {
                const format = {
                    start: "[",
                    end: "](",
                    extra: ")"
                };
                root.formattingSelected(format, root.selectionStart, root.selectionEnd);
                root.close();
            }

            QQC2.ToolTip.text: text
            QQC2.ToolTip.visible: hovered
            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        }
    }
}
