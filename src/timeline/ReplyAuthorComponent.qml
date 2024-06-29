// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

RowLayout {
    id: root

    /**
     * @brief The message author.
     *
     * This should consist of the following:
     *  - id - The matrix ID of the author.
     *  - isLocalUser - Whether the author is the local user.
     *  - avatarSource - The mxc URL for the author's avatar in the current room.
     *  - avatarMediaId - The media ID of the author's avatar.
     *  - avatarUrl - The mxc URL for the author's avatar.
     *  - displayName - The display name of the author.
     *  - display - The name of the author.
     *  - color - The color for the author.
     *  - object - The Quotient::User object for the author.
     *
     * @sa Quotient::User
     */
    required property var author

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth

    implicitHeight: Math.max(replyAvatar.implicitHeight, replyName.implicitHeight)
    spacing: Kirigami.Units.largeSpacing

    KirigamiComponents.Avatar {
        id: replyAvatar

        implicitWidth: Kirigami.Units.iconSizes.small
        implicitHeight: Kirigami.Units.iconSizes.small

        source: root.author.avatarUrl
        name: root.author.displayName
        color: root.author.color
    }
    QQC2.Label {
        id: replyName
        Layout.fillWidth: true

        color: root.author.color
        text: root.author.disambiguatedName
        elide: Text.ElideRight
    }
}
