// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick

import org.kde.kirigami as Kirigami

import org.kde.neochat

TimelineDelegate {
    id: root
    contentItem: Kirigami.PlaceholderMessage {
        text: i18n("Loading…")
    }
}
