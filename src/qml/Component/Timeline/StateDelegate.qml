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

    // extraWidth defines how the delegate can grow after the listView gets very wide
    readonly property int extraWidth: messageListView.width >= Kirigami.Units.gridUnit * 46 ? Math.min((messageListView.width - Kirigami.Units.gridUnit * 46), Kirigami.Units.gridUnit * 20) : 0
    readonly property int delegateMaxWidth: Config.compactLayout ? messageListView.width: Math.min(messageListView.width, Kirigami.Units.gridUnit * 40 + extraWidth)

    width: delegateMaxWidth

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
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: Kirigami.Units.iconSizes.small

                        name: modelData.displayName
                        source: modelData.avatarMediaId ? ("image://mxc/" + modelData.avatarMediaId) : ""
                        color: modelData.color
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
                avatar: modelData.author.avatarMediaId ? ("image://mxc/" + modelData.author.avatarMediaId) : ""
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
        }
    }
}
