// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.neochat 1.0

ColumnLayout {
    id: chatBox

    signal messageSent()

    property alias chatBar: chatBar

    readonly property int extraWidth: width >= Kirigami.Units.gridUnit * 47 ? Math.min((width - Kirigami.Units.gridUnit * 47), Kirigami.Units.gridUnit * 20) : 0
    readonly property int chatBoxMaxWidth: Config.compactLayout ? width : Math.min(width, Kirigami.Units.gridUnit * 39 + extraWidth)

    spacing: 0

    Kirigami.InlineMessage {
        Layout.fillWidth: true
        Layout.leftMargin: 1 // So we can see the border
        Layout.rightMargin: 1 // So we can see the border

        text: i18n("NeoChat is offline. Please check your network connection.")
        visible: !Controller.isOnline
    }

    Kirigami.Separator {
        Layout.fillWidth: true
    }

    ChatBar {
        id: chatBar

        visible: currentRoom.canSendEvent("m.room.message")

        Layout.fillWidth: true
        Layout.minimumHeight: implicitHeight + Kirigami.Units.largeSpacing
        // lineSpacing is height+leading, so subtract leading once since leading only exists between lines.
        Layout.maximumHeight: chatBarFontMetrics.lineSpacing * 8 - chatBarFontMetrics.leading + textField.topPadding + textField.bottomPadding

        FontMetrics {
            id: chatBarFontMetrics
            font: chatBar.textField.font
        }

        onMessageSent: {
            chatBox.messageSent();
        }
    }
}
