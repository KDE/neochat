// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtGraphicalEffects 1.12

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Dialog 1.0

QQC2.ItemDelegate {
    default property alias innerObject : column.children
    readonly property bool sentByMe: author.isLocalUser
    readonly property bool darkBackground: !sentByMe
    readonly property bool replyVisible: reply ?? false
    readonly property bool failed: marks == EventStatus.SendingFailed
    readonly property color authorColor: eventType == "notice" ? Kirigami.Theme.activeTextColor : author.color
    readonly property color replyAuthorColor: replyVisible ? reply.author.color : Kirigami.Theme.focusColor

    property bool isEmote: false
    property bool cardBackground: true
    property bool isLoaded

    property var hoverComponent

    signal saveFileAs()
    signal openExternally()
    signal replyClicked(string eventID)

    topPadding: 0
    bottomPadding: 0

    //height: mainColumn.childrenRect.height + (readMarker ? Kirigami.Units.smallSpacing : 0)
    //height: mainColumn.implicitHeight + (readMarker ? Kirigami.Units.smallSpacing : 0)

    // show hover actions
    onHoveredChanged: {
        if (hovered && !Kirigami.Settings.isMobile) {
            updateHoverComponent();
        }
    }

    // updates the global hover component to point to this delegate, and update its position
    function updateHoverComponent() {
        hoverComponent.bubble = bubble
        hoverComponent.updateFunction = updateHoverComponent;
        hoverComponent.event = model
    }

    contentItem: ColumnLayout {
        id: mainColumn
        spacing: 0

        SectionDelegate {
            id: sectionDelegate
            Layout.maximumWidth: parent.width
            Layout.alignment: Qt.AlignHCenter

            visible: showSection
        }

        RowLayout {
            id: root

            spacing: Kirigami.Units.smallSpacing
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing
            Layout.bottomMargin: 0
            Layout.topMargin: model.showAuthor ? Kirigami.Units.smallSpacing : 0

            Kirigami.Avatar {
                Layout.minimumWidth: Kirigami.Units.gridUnit * 2
                Layout.minimumHeight: Kirigami.Units.gridUnit * 2
                Layout.maximumWidth: Kirigami.Units.gridUnit * 2
                Layout.maximumHeight: Kirigami.Units.gridUnit * 2

                Layout.alignment: Qt.AlignTop

                visible: model.showAuthor && Config.showAvatarInTimeline
                name: author.name ?? author.displayName
                source: author.avatarMediaId ? ("image://mxc/" + author.avatarMediaId) : ""
                color: author.color

                Component {
                    id: userDetailDialog

                    UserDetailDialog {}
                }

                MouseArea {
                    anchors.fill: parent
                    onClicked: userDetailDialog.createObject(QQC2.ApplicationWindow.overlay, {"room": currentRoom, "user": author.object, "displayName": author.displayName, "avatarMediaId": author.avatarMediaId, "avatarUrl": author.avatarUrl}).open()
                    cursorShape: Qt.PointingHandCursor
                }
            }

            Item {
                Layout.minimumWidth: Kirigami.Units.gridUnit * 2
                Layout.preferredHeight: 1
                visible: !model.showAuthor && Config.showAvatarInTimeline
            }

            QQC2.ItemDelegate {
                id: bubble
                Layout.maximumWidth: mainColumn.width - Kirigami.Units.gridUnit * 2 - Kirigami.Units.largeSpacing * 2
                implicitHeight: contentItem.implicitHeight + topPadding
                topPadding: Kirigami.Units.largeSpacing
                bottomPadding: 0
                leftPadding: 0
                rightPadding: 0
                hoverEnabled: true

                contentItem: ColumnLayout {
                    id: column
                    spacing: 0
                    Loader {
                        id: topRow
                        active: model.showAuthor && !isEmote
                        visible: active
                        Layout.fillWidth: true
                        Layout.leftMargin: Kirigami.Units.largeSpacing
                        Layout.rightMargin: Kirigami.Units.largeSpacing
                        Layout.bottomMargin: visible ? Kirigami.Units.smallSpacing : 0

                        sourceComponent: RowLayout {
                            id: rowLayout

                            QQC2.Label {
                                Layout.fillWidth: true
                                topInset: 0

                                visible: model.showAuthor && !isEmote

                                text: author.displayName
                                font.weight: Font.Bold
                                color: author.color
                                wrapMode: Text.Wrap
                            }
                            QQC2.Label {
                                visible: model.showAuthor && !isEmote
                                text: time.toLocaleTimeString(Locale.ShortFormat)
                                color: Kirigami.Theme.disabledTextColor
                            }
                        }
                    }
                    Loader {
                        id: replyLoader
                        source: 'qrc:imports/NeoChat/Component/Timeline/ReplyComponent.qml'
                        active: replyVisible
                        visible: active
                        Layout.bottomMargin: Kirigami.Units.smallSpacing

                        Connections {
                            target: replyLoader.item
                            function onClicked() {
                                replyClicked(reply.eventId)
                            }
                        }
                    }
                }

                background: Kirigami.ShadowedRectangle {
                    visible: cardBackground
                    color: model.isHighlighted ? Kirigami.Theme.positiveBackgroundColor : Kirigami.Theme.backgroundColor
                    radius: Kirigami.Units.smallSpacing
                    shadow.size: Kirigami.Units.smallSpacing
                    shadow.color: !model.isHighlighted ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                    border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                    border.width: Kirigami.Units.devicePixelRatio
                }
            }
        }
        Loader {
            id: loader
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing * 2
            Layout.topMargin: active ? Kirigami.Units.smallSpacing : 0
            //Layout.bottomMargin: readMarker ? Kirigami.Units.smallSpacing : 0
            active: eventType !== "state" && eventType !== "notice" && reaction != undefined && reaction.length > 0
            visible: active
            sourceComponent: ReactionDelegate { }
        }
    }

    background: Item {
        Rectangle {
            width: parent.width * 0.9
            x: parent.width * 0.05
            height: Kirigami.Units.smallSpacing / 2
            anchors.bottom: parent.bottom
            visible: readMarker
            color: Kirigami.Theme.positiveTextColor
        }
    }
}
