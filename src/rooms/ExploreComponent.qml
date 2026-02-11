// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtMultimedia

import org.kde.kirigami as Kirigami

import org.kde.neochat

RowLayout {
    id: root

    required property NeoChatConnection connection
    property bool collapsed: false

    signal search

    /**
     * @brief Emitted when the text is changed in the search field.
     */
    signal textChanged(string newText)

    Kirigami.Heading {
        Layout.fillWidth: true
        // Roughly equivalent to what Kirigami does for its built-in headings
        Layout.leftMargin: Kirigami.Units.gridUnit - Kirigami.Units.mediumSpacing
        visible: !root.collapsed
        text: {
            if (Kirigami.Settings.isMobile) {
                if (RoomManager.currentSpace === '') {
                    return i18nc("@title Home space", "Home");
                } else if(RoomManager.currentSpace === 'DM') {
                    return i18nc("@title", "Direct Messages");
                }
                return root.connection.room(RoomManager.currentSpace)?.displayName;
            }
            return i18nc("@title List of rooms", "Rooms");
        }
    }
    Item {
        Layout.fillWidth: true
        visible: root.collapsed
    }

    QQC2.ToolButton {
        id: searchButton
        display: QQC2.AbstractButton.IconOnly
        onClicked: root.search();
        icon.name: "search"
        text: i18nc("@action", "Search Rooms")

        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }

    QQC2.ToolButton {
        display: QQC2.Button.IconOnly
        visible: Kirigami.Settings.isMobile
        text: i18nc("@action:button", "Open Settings")
        icon.name: "settings-configure-symbolic"
        onClicked: NeoChatSettingsView.open()

        QQC2.ToolTip.text: text
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
    }
}
