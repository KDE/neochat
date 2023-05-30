// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

QQC2.Control {
    id: stateDelegate

    readonly property bool sectionVisible: model.showSection

    width: stateDelegateSizeHelper.currentWidth

    state: Config.compactLayout ? "alignLeft" : "alignCenter"
    // Align left when in compact mode and center when using bubbles
    states: [
        State {
            name: "alignLeft"
            AnchorChanges {
                target: stateDelegate
                anchors.horizontalCenter: undefined
                anchors.left: parent ? parent.left : undefined
            }
        },
        State {
            name: "alignCenter"
            AnchorChanges {
                target: stateDelegate
                anchors.horizontalCenter: parent ? parent.horizontalCenter : undefined
                anchors.left: undefined
            }
        }
    ]

    transitions: [
        Transition {
            AnchorAnimation{duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic}
        }
    ]

    height: columnLayout.implicitHeight + columnLayout.anchors.topMargin

    ColumnLayout {
        id: columnLayout

        property bool folded: true

        spacing: sectionVisible ? Kirigami.Units.largeSpacing : 0
        anchors.top: parent.top
        anchors.topMargin: sectionVisible ? 0 : Kirigami.Units.largeSpacing
        anchors.left: parent.left
        anchors.right: parent.right

        SectionDelegate {
            id: sectionDelegate
            Layout.fillWidth: true
            visible: sectionVisible
            labelText: sectionVisible ? section : ""
        }
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.gridUnit * 1.5 + Kirigami.Units.smallSpacing * 1.5 + (Config.compactLayout ? Kirigami.Units.largeSpacing * 1.25 : 0)
            Layout.rightMargin: Kirigami.Units.largeSpacing
            visible: stateEventRepeater.count !== 1

            Flow {
                visible: columnLayout.folded
                spacing: -Kirigami.Units.iconSizes.small / 2
                Repeater {
                    model: authorList
                    delegate: Kirigami.Avatar {
                        topInset: Kirigami.Units.smallSpacing / 2
                        topPadding: Kirigami.Units.smallSpacing / 2
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small + Kirigami.Units.smallSpacing / 2

                        name: modelData.displayName
                        source: modelData.avatarSource
                        color: modelData.color
                    }
                }
                QQC2.Label {
                    id: excessAuthorsLabel
                    text: model.excessAuthors
                    visible: model.excessAuthors !== ""
                    color: Kirigami.Theme.textColor
                    horizontalAlignment: Text.AlignHCenter
                    background: Kirigami.ShadowedRectangle {
                        color: Kirigami.Theme.backgroundColor
                        Kirigami.Theme.inherit: false
                        Kirigami.Theme.colorSet: Kirigami.Theme.View
                        radius: height / 2
                        shadow.size: Kirigami.Units.smallSpacing
                        shadow.color: Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                        border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                        border.width: 1
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
                visible: columnLayout.folded

                text: aggregateDisplay
                elide: Qt.ElideRight
                textFormat: Text.RichText
                wrapMode: Text.WordWrap
                onLinkActivated: RoomManager.openResource(link)
            }
            Item {
                Layout.fillWidth: true
                visible: !columnLayout.folded
            }
            QQC2.ToolButton {
                icon.name: (!columnLayout.folded ? "go-up" : "go-down")
                icon.width: Kirigami.Units.iconSizes.small
                icon.height: Kirigami.Units.iconSizes.small

                onClicked: {
                    columnLayout.toggleFolded()
                }
            }
        }
        Repeater {
            id: stateEventRepeater
            model: stateEvents
            delegate: StateComponent {
                Layout.fillWidth: true
                Layout.leftMargin: Kirigami.Units.gridUnit * 1.5 + Kirigami.Units.smallSpacing * 1.5 + (Config.compactLayout ? Kirigami.Units.largeSpacing * 1.25 : 0)
                Layout.rightMargin: Kirigami.Units.largeSpacing
                visible: !columnLayout.folded || stateEventRepeater.count === 1

                name: modelData.author.displayName
                avatar: modelData.author.avatarSource
                color: modelData.author.color
                text: `<style>a {text-decoration: none;}</style><a href="https://matrix.to/#/${modelData.author.id}" style="color: ${modelData.author.color}">${modelData.authorDisplayName}</a> ${modelData.text}`

                onAvatarClicked: RoomManager.openResource("https://matrix.to/#/" + modelData.author.id)
                onLinkClicked: RoomManager.openResource(link)
            }
        }

        function toggleFolded() {
            folded = !folded
            foldedChanged()
        }
        AvatarFlow {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: Kirigami.Units.largeSpacing
            visible: showReadMarkers
            model: readMarkers
            toolTipText: readMarkersString
            excessAvatars: excessReadMarkers
        }
    }

    DelegateSizeHelper {
        id: stateDelegateSizeHelper
        startBreakpoint: Kirigami.Units.gridUnit * 46
        endBreakpoint: Kirigami.Units.gridUnit * 66
        startPercentWidth: 100
        endPercentWidth: Config.compactLayout ? 100 : 85
        maxWidth: Config.compactLayout ? -1 : Kirigami.Units.gridUnit * 60

        parentWidth: stateDelegate.parent ? stateDelegate.parent.width : 0
    }
}
