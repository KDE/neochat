// SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Control {
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
            id: rowLayout
            implicitHeight: label.contentHeight
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.gridUnit * 1.5 + Kirigami.Units.smallSpacing * 1.5 + (Config.compactLayout ? Kirigami.Units.largeSpacing * 1.25 : 0)
            Layout.rightMargin: Kirigami.Units.largeSpacing

            Kirigami.Avatar {
                id: icon
                Layout.preferredWidth: Kirigami.Units.iconSizes.small
                Layout.preferredHeight: Kirigami.Units.iconSizes.small

                name: author.displayName
                source: author.avatarMediaId ? ("image://mxc/" + author.avatarMediaId) : ""
                color: author.color

                Component {
                    id: userDetailDialog

                    UserDetailDialog {}
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: userDetailDialog.createObject(ApplicationWindow.overlay, {room: currentRoom, user: author.object, displayName: author.displayName, avatarMediaId: author.avatarMediaId, avatarUrl: author.avatarUrl}).open()
                }
            }

            Label {
                id: label
                Layout.alignment: Qt.AlignVCenter
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                textFormat: Text.RichText
                text: `<style>a {text-decoration: none;}</style><a href="https://matrix.to/#/${author.id}" style="color: ${author.color}">${currentRoom.htmlSafeMemberName(author.id)}</a> ${aggregateDisplay}`
                onLinkActivated: userDetailDialog.createObject(ApplicationWindow.overlay, {room: currentRoom, user: author.object, displayName: author.displayName, avatarMediaId: author.avatarMediaId, avatarUrl: author.avatarUrl}).open()
            }
        }
    }
}
