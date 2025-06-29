// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

/**
 * @brief A component for a hotel itinerary reservation.
 */
ItineraryReservationComponent {
    id: root

    /**
     * @brief The name of the reservation.
     */
    required property string name

    /**
     * @brief The check-in time at the hotel.
     *
     * Includes date.
     */
    required property string startTime

    /**
     * @brief The check-out time at the hotel.
     *
     * Includes date.
     */
    required property string endTime

    /**
     * @brief The address of the hotel.
     */
    required property string address

    headerItem: RowLayout {
        TransportIcon {
            source: "go-home-symbolic"
            isMask: true
            size: Kirigami.Units.iconSizes.smallMedium
        }
        QQC2.Label {
            text: root.name
            elide: Text.ElideRight

            Accessible.ignored: true
        }
    }

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        QQC2.Label {
            Layout.fillWidth: true
            visible: text !== ""
            text: root.address
            wrapMode: Text.WordWrap
        }
        QQC2.Label {
            text: i18n("Check-in time: %1", root.startTime)
            color: Kirigami.Theme.textColor
        }
        QQC2.Label {
            text: i18n("Check-out time: %1", root.endTime)
            color: Kirigami.Theme.textColor
        }

    }
}
