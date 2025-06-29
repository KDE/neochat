// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show that part of a message is loading.
 */
RowLayout {
    id: root

    required property string display

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth
    spacing: Kirigami.Units.smallSpacing

    QQC2.BusyIndicator {}
    Kirigami.Heading {
        id: loadingText
        Layout.fillWidth: true
        verticalAlignment: Text.AlignVCenter
        level: 2
        text: root.display.length > 0 ? root.display : i18n("Loading")
    }
}

