// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami

/**
 * @brief A component to show a preview of a file that can integrate with KDE itinerary.
 */
ColumnLayout {
    id: root

    /**
     * @brief A model with the itinerary preview of the file.
     */
    required property var itineraryModel

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth
    spacing: Kirigami.Units.largeSpacing

    Repeater {
        id: itinerary
        model: root.itineraryModel
        onModelChanged: console.warn(itinerary.count)
        delegate: DelegateChooser {
            role: "type"
            DelegateChoice {
                roleValue: "TrainReservation"
                delegate: ColumnLayout {
                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }
                    RowLayout {
                        QQC2.Label {
                            text: model.name
                        }
                        QQC2.Label {
                            text: model.coach ? i18n("Coach: %1, Seat: %2", model.coach, model.seat) : ""
                            visible: model.coach
                            opacity: 0.7
                        }
                    }
                    RowLayout {
                        Layout.fillWidth: true
                        ColumnLayout {
                            QQC2.Label {
                                text: model.departureStation + (model.departurePlatform ? (" [" + model.departurePlatform + "]") : "")
                            }
                            QQC2.Label {
                                text: model.departureTime
                                opacity: 0.7
                            }
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                        ColumnLayout {
                            QQC2.Label {
                                text: model.arrivalStation + (model.arrivalPlatform ? (" [" + model.arrivalPlatform + "]") : "")
                            }
                            QQC2.Label {
                                text: model.arrivalTime
                                opacity: 0.7
                                Layout.alignment: Qt.AlignRight
                            }
                        }
                    }
                }
            }
            DelegateChoice {
                roleValue: "LodgingReservation"
                delegate: ColumnLayout {
                    Kirigami.Separator {
                        Layout.fillWidth: true
                    }
                    QQC2.Label {
                        text: model.name
                    }
                    QQC2.Label {
                        text: i18nc("<start time> - <end time>", "%1 - %2", model.startTime, model.endTime)
                    }
                    QQC2.Label {
                        text: model.address
                    }
                }
            }
        }
    }
    QQC2.Button {
        icon.name: "map-globe"
        text: i18nc("@action", "Send to KDE Itinerary")
        QQC2.ToolTip.visible: hovered
        QQC2.ToolTip.text: text
        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
        onClicked: itineraryModel.sendToItinerary()
        visible: itinerary.count > 0
    }
}
