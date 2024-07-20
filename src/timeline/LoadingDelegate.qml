// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick

import org.kde.kirigami as Kirigami

import org.kde.neochat

TimelineDelegate {
    id: root

    width: parent?.width
    rightPadding: NeoChatConfig.compactLayout && root.ListView.view.width >= Kirigami.Units.gridUnit * 20 ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing

    alwaysFillWidth: NeoChatConfig.compactLayout

    contentItem: Kirigami.PlaceholderMessage {
        text: i18n("Loading…")
    }
}
