// SPDX-FileCopyrightText: 2021 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.neochat

/**
 * @brief A timeline delegate for an encrypted message that can't be decrypted.
 *
 * @inherit MessageDelegate
 */
MessageDelegate {
    id: encryptedDelegate

    bubbleContent: TextEdit {
        text: i18n("This message is encrypted and the sender has not shared the key with this device.")
        color: Kirigami.Theme.disabledTextColor
        selectedTextColor: Kirigami.Theme.highlightedTextColor
        selectionColor: Kirigami.Theme.highlightColor
        font.pointSize: Kirigami.Theme.defaultFont.pointSize
        selectByMouse: !Kirigami.Settings.isMobile
        readOnly: true
        wrapMode: Text.WordWrap
        textFormat: Text.RichText
        Layout.leftMargin: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing : 0
    }
}
