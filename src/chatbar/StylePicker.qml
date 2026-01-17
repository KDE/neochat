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

    y: -implicitHeight
    padding: Kirigami.Units.largeSpacing

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        Repeater {
            model: 9

            delegate: StyleDelegate {
                id: styleDelegate
                required property int index

                Layout.fillWidth: true
                Layout.minimumWidth: Kirigami.Units.gridUnit * 8
                Layout.minimumHeight: Kirigami.Units.gridUnit * 2

                style: index
                highlight: root.chatButtonHelper.currentStyle === index || hovered

                onPressed: (event) => {
                    if (index === LibNeoChat.RichFormat.Paragraph ||
                        index === LibNeoChat.RichFormat.Code ||
                        index === LibNeoChat.RichFormat.Quote
                    ) {
                        root.chatContentModel.insertStyleAtCursor(styleDelegate.index);
                    } else {
                        root.chatButtonHelper.setFormat(styleDelegate.index);
                    }
                    root.close();
                }
            }
        }
    }

    background: Kirigami.ShadowedRectangle {
        Kirigami.Theme.inherit: false
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        radius: Kirigami.Units.cornerRadius
        color: Kirigami.Theme.backgroundColor
        border {
            width: 1
            color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
        }
    }
}
