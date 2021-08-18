// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.neochat 1.0

TextEdit {
    text: i18n("This message is encrypted and the sender has not shared the key with this device.")
    color: Kirigami.Theme.disabledTextColor
    font.pointSize: Kirigami.Theme.defaultFont.pointSize
    selectByMouse: !Kirigami.Settings.isMobile
    readOnly: true
    wrapMode: Text.WordWrap
    textFormat: Text.RichText
    width: messageContainer.bubbleMaxWidth
}
