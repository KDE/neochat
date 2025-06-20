// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

/**
 * @brief A timeline delegate for visualising an aggregated list of consecutive state events.
 *
 * @inherit TimelineDelegate
 */
TimelineDelegate {
    id: root

    /**
     * @brief List of the first 5 unique authors of the aggregated state event.
     */
    required property var authorList

    /**
     * @brief The number of unique authors beyond the first 5.
     */
    required property string excessAuthors

    /**
     * @brief Single line aggregation of all the state events.
     */
    required property string aggregateDisplay

    /**
     * @brief List of state events in the aggregated state.
     */
    required property var stateEvents

    /**
     * @brief Whether the section header should be shown.
     */
    required property bool showSection

    /**
     * @brief The date of the event as a string.
     */
    required property string section

    /**
     * @brief A model with the first 5 other user read markers for this message.
     */
    required property var readMarkers

    /**
     * @brief Whether the other user read marker component should be shown.
     */
    required property bool showReadMarkers

    /**
     * @brief Whether the state event is folded to a single line.
     */
    property bool folded: true

    width: parent?.width
    rightPadding: NeoChatConfig.compactLayout && root.ListView.view.width >= Kirigami.Units.gridUnit * 20 ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing

    alwaysFillWidth: NeoChatConfig.compactLayout

    contentItem: ColumnLayout {
        Kirigami.ListSectionHeader {
            Layout.fillWidth: true
            visible: root.showSection
            horizontalPadding: 0
            text: root.section
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing * 1.5
            Layout.rightMargin: Kirigami.Units.largeSpacing * 1.5
            Layout.topMargin: Kirigami.Units.largeSpacing
            visible: stateEventRepeater.count !== 1

            Flow {
                visible: root.folded
                spacing: -Kirigami.Units.iconSizes.small / 2

                Repeater {
                    model: root.authorList
                    delegate: Item {
                        id: avatarDelegate

                        required property var modelData

                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing / 2

                        KirigamiComponents.Avatar {
                            y: Kirigami.Units.smallSpacing / 2

                            implicitWidth: Kirigami.Units.iconSizes.small
                            implicitHeight: Kirigami.Units.iconSizes.small

                            name: parent.modelData.displayName
                            source: parent.modelData.avatarUrl
                            color: parent.modelData.color
                            asynchronous: true
                        }
                    }
                }

                QQC2.Label {
                    id: excessAuthorsLabel

                    text: root.excessAuthors
                    visible: root.excessAuthors !== ""
                    color: Kirigami.Theme.textColor
                    horizontalAlignment: Text.AlignHCenter
                    background: Kirigami.ShadowedRectangle {
                        Kirigami.Theme.inherit: false
                        Kirigami.Theme.colorSet: Kirigami.Theme.View

                        color: Kirigami.Theme.backgroundColor
                        radius: height / 2

                        shadow {
                            size: Kirigami.Units.smallSpacing
                            color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                        }

                        border {
                            color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                            width: 1
                        }
                    }

                    height: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing
                    width: Math.max(excessAuthorsTextMetrics.advanceWidth + Kirigami.Units.smallSpacing * 2, height)

                    TextMetrics {
                        id: excessAuthorsTextMetrics
                        text: excessAuthorsLabel.text
                    }
                }
            }
            QQC2.Label {
                Layout.fillWidth: true
                visible: root.folded

                text: root.aggregateDisplay
                textFormat: Text.StyledText
                elide: Text.ElideRight
                onLinkActivated: RoomManager.resolveResource(link)
                HoverHandler {
                    cursorShape: parent.hoveredLink ? Qt.PointingHandCursor : Qt.IBeamCursor
                }
            }
            Item {
                Layout.fillWidth: true
                implicitHeight: foldButton.implicitHeight
                visible: !root.folded
            }
            QQC2.ToolButton {
                id: foldButton
                icon {
                    name: (!root.folded ? "go-up" : "go-down")
                    width: Kirigami.Units.iconSizes.small
                    height: Kirigami.Units.iconSizes.small
                }

                onClicked: root.toggleFolded()
            }
        }
        Repeater {
            id: stateEventRepeater
            model: root.stateEvents
            delegate: StateComponent {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.largeSpacing * 1.5
                Layout.rightMargin: Kirigami.Units.largeSpacing * 1.5
                Layout.topMargin: Kirigami.Units.largeSpacing
                visible: !root.folded || stateEventRepeater.count === 1
            }
        }
        AvatarFlow {
            Layout.alignment: Qt.AlignRight
            visible: root.showReadMarkers
            model: root.readMarkers
        }
    }

    function toggleFolded() {
        folded = !folded;
        foldedChanged();
    }
}
