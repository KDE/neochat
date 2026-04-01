// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick

import org.kde.kirigami as Kirigami

Item {
    width: parent?.width
    // Make headroom for the standalone typing indicator on mobile. On desktop it takes up space in the chat bar footer instead.
    height: Kirigami.Settings.isMobile ? Kirigami.Units.largeSpacing + Math.round(Kirigami.Theme.defaultFont.pointSize * 2) : Kirigami.Units.largeSpacing
}
