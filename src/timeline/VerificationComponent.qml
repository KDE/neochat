// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

RowLayout {
    id: root

    required property NeoChatRoom room

    Kirigami.Icon {
        source: "security-high"
    }
    QQC2.Label {
        //FIXME: the event might have been sent by us.
        text: i18n("%1 started a user verification", room.directChatRemoteUser.displayName)
    }
}
