// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

/**
 * @brief A component for a train itinerary reservation component.
 */
ItineraryReservationComponent {
    id: root

    /**
     * @brief The name of the reservation.
     */
    required property string name

    /**
     * @brief The departure time of the train.
     */
    required property string departureTime

    /**
     * @brief The departure station of the train.
     */
    required property string departureLocation

    /**
     * @brief The address of the departure station of the train.
     */
    required property string departureAddress

    /**
     * @brief The departure platform of the train.
     */
    required property string departurePlatform

    /**
     * @brief The arrival time of the train.
     */
    required property string arrivalTime

    /**
     * @brief The arrival station of the train.
     */
    required property string arrivalLocation

    /**
     * @brief The address of the arrival station of the train.
     */
    required property string arrivalAddress

    /**
     * @brief The arrival platform of the train.
     */
    required property string arrivalPlatform

    headerItem: RowLayout {
        TransportIcon {
            source: "qrc:/qt/qml/org/kde/neochat/timeline/images/train.svg"
            isMask: true
            size: Kirigami.Units.iconSizes.smallMedium
        }
        QQC2.Label {
            Layout.fillWidth: true
            text: root.name
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignLeft

            Accessible.ignored: true
        }
        QQC2.Label {
            text: root.departureTime
        }
    }

    contentItem: ColumnLayout {
        spacing: 0

        RowLayout {
            Layout.fillWidth: true
            ColumnLayout{
                id: lineSectionColumn
                spacing: 0
                JourneySectionStopDelegateLineSegment {
                    Layout.fillHeight: true
                    isDeparture: true
                }
                JourneySectionStopDelegateLineSegment {
                    visible: departureCountryLayout.visible
                    Layout.fillHeight: true
                }
            }

            ColumnLayout{
                Layout.bottomMargin:  Kirigami.Units.largeSpacing

                spacing:0
                Layout.fillHeight: true
                Layout.fillWidth: true
                RowLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    QQC2.Label {
                        id: depTime
                        text: root.departureTime
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: root.departureLocation
                        elide: Text.ElideRight
                    }
                    QQC2.Label {
                        Layout.alignment: Qt.AlignRight
                        text: {
                            let platform = root.departurePlatform;

                            if (platform) {
                                return i18n("Pl. %1", platform);
                            } else {
                                return "";
                            }
                        }
                    }
                }
                RowLayout{
                    id: departureCountryLayout
                    visible: departureCountryLabel.text.length > 0
                    Item{
                        Layout.minimumWidth: depTime.width
                    }
                    QQC2.Label {
                        id: departureCountryLabel
                        Layout.fillWidth: true
                        text: root.departureAddress
                    }
                }
            }
        }
        RowLayout{
            Layout.fillWidth: true
            JourneySectionStopDelegateLineSegment {
                Layout.fillHeight: true
            }
            Kirigami.Separator {
                Layout.fillWidth: true
            }
        }
        RowLayout {
            Layout.fillWidth: true
            ColumnLayout {
                spacing: 0
                JourneySectionStopDelegateLineSegment {
                    visible: arrivalCountryLayout.visible
                    Layout.fillHeight: true
                }
                JourneySectionStopDelegateLineSegment {
                    Layout.fillHeight: true
                    isArrival: true
                }
            }
            ColumnLayout{
                Layout.topMargin: Kirigami.Units.largeSpacing

                spacing:0
                Layout.fillHeight: true
                Layout.fillWidth: true
                RowLayout {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    QQC2.Label {
                        text: root.arrivalTime
                    }
                    QQC2.Label {
                        Layout.fillWidth: true
                        font.bold: true
                        text: root.arrivalLocation
                        elide: Text.ElideRight
                    }
                    QQC2.Label {
                        Layout.alignment: Qt.AlignRight
                        text: {
                            let platform = root.arrivalPlatform;

                            if (platform) {
                                return i18n("Pl. %1", platform);
                            } else {
                                return "";
                            }
                        }
                    }
                }
                RowLayout {
                    id: arrivalCountryLayout
                    visible: arrivalCountryLabel.text.length > 0
                    Item{
                        Layout.minimumWidth: depTime.width
                    }
                    QQC2.Label {
                        id: arrivalCountryLabel
                        Layout.fillWidth: true
                        text: root.arrivalAddress
                    }
                }
            }
        }
    }
}

