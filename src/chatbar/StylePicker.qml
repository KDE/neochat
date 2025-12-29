// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat.libneochat as LibNeoChat
import org.kde.neochat.messagecontent as MessageContent

QQC2.Popup {
    id: root

    required property MessageContent.ChatBarMessageContentModel chatContentModel
    required property ChatButtonHelper chatButtonHelper
    readonly property LibNeoChat.ChatDocumentHandler focusedDocumentHandler: chatContentModel.focusedDocumentHandler

    y: -implicitHeight

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Repeater {
            model: 9

            delegate: QQC2.TextArea {
                id: styleDelegate
                required property int index

                Layout.fillWidth: true
                Layout.minimumWidth: Kirigami.Units.gridUnit * 7
                Layout.minimumHeight: Kirigami.Units.gridUnit * 2
                leftPadding: lineRow.visible ? lineRow.width + lineRow.anchors.leftMargin + Kirigami.Units.smallSpacing : Kirigami.Units.largeSpacing
                verticalAlignment: Text.AlignVCenter

                enabled: root.chatContentModel.focusType !== LibNeoChat.MessageComponentType.Code || styleDelegate.index === LibNeoChat.RichFormat.Paragraph || styleDelegate.index === LibNeoChat.RichFormat.Quote
                readOnly: true
                selectByMouse: false

                onPressed: (event) => {
                    if (styleDelegate.index === LibNeoChat.RichFormat.Paragraph ||
                        styleDelegate.index === LibNeoChat.RichFormat.Code ||
                        styleDelegate.index === LibNeoChat.RichFormat.Quote
                    ) {
                        root.chatContentModel.insertStyleAtCursor(styleDelegate.index);
                    } else {
                        root.chatButtonHelper.setFormat(styleDelegate.index);
                    }
                    root.close();
                }

                RowLayout {
                    id: lineRow
                    anchors {
                        top: styleDelegate.top
                        bottom: styleDelegate.bottom
                        left: styleDelegate.left
                        leftMargin: Kirigami.Units.smallSpacing
                    }

                    visible: styleDelegate.index === LibNeoChat.RichFormat.Code

                    QQC2.Label {
                        horizontalAlignment: Text.AlignRight
                        verticalAlignment: Text.AlignVCenter
                        text: "1"
                        color: Kirigami.Theme.disabledTextColor

                        font.family: "monospace"
                    }
                    Kirigami.Separator {
                        Layout.fillHeight: true
                    }
                }

                StyleDelegateHelper {
                    textItem: styleDelegate
                }

                background: Rectangle {
                    color: Kirigami.Theme.backgroundColor
                    Kirigami.Theme.colorSet: styleDelegate.index === LibNeoChat.RichFormat.Quote ? Kirigami.Theme.Window : Kirigami.Theme.View
                    Kirigami.Theme.inherit: false
                    radius: Kirigami.Units.cornerRadius
                    border {
                        width: 1
                        color: styleDelegate.hovered || (root.focusedDocumentHandler?.style ?? false) === styleDelegate.index ?
                               Kirigami.Theme.highlightColor :
                               Kirigami.ColorUtils.linearInterpolation(
                                   Kirigami.Theme.backgroundColor,
                                   Kirigami.Theme.textColor,
                                   Kirigami.Theme.frameContrast
                               )
                    }
                }
            }
        }
    }

    background: Kirigami.ShadowedRectangle {
        radius: Kirigami.Units.cornerRadius
        color: Kirigami.Theme.backgroundColor

        border {
            width: 1
            color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
        }

        shadow {
            size: Kirigami.Units.gridUnit
            yOffset: 0
            color: Qt.rgba(0, 0, 0, 0.2)
        }

        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View
    }
}
