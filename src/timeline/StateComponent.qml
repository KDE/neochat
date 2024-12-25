// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

/**
 * @brief A component for visualising a single state event
 */
RowLayout {
    id: root

    /**
     * @brief All model roles as a map with the property names as the keys.
     */
    required property var modelData

    /**
     * @brief The message author.
     *
     * A Quotient::RoomMember object.
     *
     * @sa Quotient::RoomMember
     */
    property var author: modelData.author

    /**
     * @brief The displayname for the event's sender; for name change events, the old displayname.
     */
    property string authorDisplayName: modelData.authorDisplayName

    /**
     * @brief The display text for the state event.
     */
    property string text: modelData.text

    KirigamiComponents.Avatar {
        id: stateAvatar

        Layout.preferredWidth: Kirigami.Units.iconSizes.small
        Layout.preferredHeight: Kirigami.Units.iconSizes.small

        source: root.author?.avatarUrl ?? ""
        name: root.author?.displayName ?? ""
        color: root.author?.color ?? Kirigami.Theme.highlightColor
        asynchronous: true

        MouseArea {
            anchors.fill: parent
            enabled: root.author
            cursorShape: Qt.PointingHandCursor
            onClicked: RoomManager.resolveResource("https://matrix.to/#/" + root.author?.id ?? "")
        }
    }

    QQC2.Label {
        id: label
        Layout.alignment: Qt.AlignVCenter
        Layout.fillWidth: true
        text: `<style>a {text-decoration: none; color: ${Kirigami.Theme.textColor};}</style><a href="https://matrix.to/#/${root.author?.id ?? ""}">${root.authorDisplayName}</a> ${root.text}`
        wrapMode: Text.WordWrap
        textFormat: Text.RichText
        onLinkActivated: link => RoomManager.resolveResource(link)
        HoverHandler {
            cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
        }
    }
}
