// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

/**
 * @brief A base component for an itinerary reservation.
 */
FormCard.FormCard {
    id: root

    /**
     * @brief An item with the header content.
     */
    property alias headerItem: headerDelegate.contentItem

    /**
     * @brief An item with the main body content.
     */
    property alias contentItem: content.contentItem

    Layout.fillWidth: true
    implicitWidth: Math.max(headerDelegate.implicitWidth, content.implicitWidth)

    Component.onCompleted: children[0].radius = Kirigami.Units.smallSpacing

    FormCard.AbstractFormDelegate {
        id: headerDelegate
        Layout.fillWidth: true
        background: Rectangle {
            color: Kirigami.Theme.backgroundColor
            Kirigami.Theme.colorSet: Kirigami.Theme.Header
            Kirigami.Theme.inherit: false
        }
    }
    FormCard.AbstractFormDelegate {
        id: content
        Layout.fillWidth: true
        background: null
    }
}

