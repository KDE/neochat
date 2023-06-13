// SPDX-FileCopyrightText: 2022 Carson Black <uhhadd@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.neochat 1.0

TimelineContainer {
    id: root

    width: ListView.view.width

    innerObject: QQC2.Control {
        Layout.leftMargin: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing : 0
        padding: Kirigami.Units.gridUnit*2

        contentItem: QQC2.Label {
            text: root.author.isLocalUser ? i18n("Outgoing Call") : i18n("Incoming Call")
        }
    }
}
