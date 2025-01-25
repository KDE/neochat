// SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates

import org.kde.neochat
import org.kde.neochat.chatbar

/**
 * @brief A component to show a reply button for threads in a message bubble.
 */
Delegates.RoundedItemDelegate {
    id: root

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    /**
     * @brief Request more events in the thread be loaded.
     */
    signal fetchMoreEvents()

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth

    leftInset: 0
    rightInset: 0

    highlighted: true

    icon.name: "arrow-up"
    icon.width: Kirigami.Units.iconSizes.sizeForLabels
    icon.height: Kirigami.Units.iconSizes.sizeForLabels
    text: i18nc("@action:button", "Fetch More Events")

    onClicked: {
        root.fetchMoreEvents()
    }

    contentItem: Kirigami.Icon {
        implicitWidth: root.icon.width
        implicitHeight: root.icon.height
        source: root.icon.name
    }
}
