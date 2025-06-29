// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

/**
 * @brief A component for a food itinerary reservation.
 */
ItineraryReservationComponent {
    id: root

    /**
     * @brief The name of the reservation.
     */
    required property string name

    /**
     * @brief The start time of the reservation.
     *
     * Includes date.
     */
    required property string startTime

    /**
     * @brief The address of the hotel.
     */
    required property string address

    headerItem: RowLayout {
        TransportIcon {
            source: "qrc:/qt/qml/org/kde/neochat/timeline/images/foodestablishment.svg"
            isMask: true
            size: Kirigami.Units.iconSizes.smallMedium
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: root.name
            elide: Text.ElideRight

            Accessible.ignored: true
        }
        QQC2.Label {
            text: root.startTime
        }
    }

    contentItem: QQC2.Label {
        visible: text !== ""
        text: root.address
        wrapMode: Text.Wrap
    }
}
