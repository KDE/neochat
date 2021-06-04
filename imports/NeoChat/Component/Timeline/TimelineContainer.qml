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
    id: messageDelegate
    default property alias innerObject : column.children
    // readonly property bool failed: marks == EventStatus.SendingFailed
    property bool isLoaded

    property bool isEmote: false
    property bool cardBackground: true

    readonly property int bubbleMaxWidth: Config.compactLayout && !Config.showAvatarInTimeline ? width : (Config.compactLayout ? width - Kirigami.Units.gridUnit * 2 - Kirigami.Units.largeSpacing * 4 : Math.min(width - Kirigami.Units.gridUnit * 2 - Kirigami.Units.largeSpacing * 6, Kirigami.Units.gridUnit * 20))

    signal saveFileAs()
    signal openExternally()
    signal replyClicked(string eventID)

    topPadding: 0
    bottomPadding: 0
    background: null
    property Item hoverComponent

    // show hover actions
    onHoveredChanged: {
        if (hovered && !Kirigami.Settings.isMobile) {
            updateHoverComponent();
        }
    }

    // updates the global hover component to point to this delegate, and update its position
    function updateHoverComponent() {
        if (hoverComponent) {
            hoverComponent.bubble = bubble
            hoverComponent.updateFunction = updateHoverComponent;
            hoverComponent.event = model
        }
    }

    height: sectionDelegate.height + Math.max(model.showAuthor ? avatar.height : 0, bubble.implicitHeight) + loader.height + (showAuthor ? Kirigami.Units.largeSpacing : 0)

    SectionDelegate {
        id: sectionDelegate
        width: parent.width
        anchors.left: avatar.left
        anchors.leftMargin: Kirigami.Units.smallSpacing
        visible: model.showSection
        height: visible ? implicitHeight : 0
    }

    Kirigami.Avatar {
        id: avatar
        width: visible || Config.showAvatarInTimeline ? Kirigami.Units.gridUnit * 2 : 0
        height: width
        sourceSize.width: width
        sourceSize.height: width
        anchors {
            top: sectionDelegate.bottom
            topMargin: model.showAuthor ? Kirigami.Units.largeSpacing : 0
            left: parent.left
            leftMargin: Kirigami.Units.largeSpacing
        }

        visible: model.showAuthor && Config.showAvatarInTimeline
        name: model.author.name ?? model.author.displayName
        source: visible && model.author.avatarMediaId ? ("image://mxc/" + model.author.avatarMediaId) : ""
        color: model.author.color

        MouseArea {
            anchors.fill: parent
            onClicked: {
                userDetailDialog.createObject(QQC2.ApplicationWindow.overlay, {
                    room: currentRoom,
                    user: author.object,
                    displayName: author.displayName,
                    avatarMediaId: author.avatarMediaId,
                    avatarUrl: author.avatarUrl
                }).open();
            }
            cursorShape: Qt.PointingHandCursor
        }
    }

    QQC2.Control {
        id: bubble
        topPadding: !Config.compactLayout ? Kirigami.Units.largeSpacing : 0
        bottomPadding: !Config.compactLayout ? Kirigami.Units.largeSpacing : 0
        leftPadding: Kirigami.Units.smallSpacing
        rightPadding: Config.compactLayout ? Kirigami.Units.largeSpacing : Kirigami.Units.smallSpacing
        hoverEnabled: true

        state: Config.compactLayout ? "compactLayout" : "default"
        anchors {
            top: avatar.top
            left: avatar.right
            leftMargin: Kirigami.Units.largeSpacing
        }
        // HACK: anchoring didn't reset anchors.right when switching from parent.right to undefined reliably
        width: Config.compactLayout ? messageDelegate.width - (Config.showAvatarInTimeline ? Kirigami.Units.gridUnit * 2 : 0) + Kirigami.Units.largeSpacing * 2 : implicitWidth

        contentItem: ColumnLayout {
            id: column
            spacing: 0
            Item {
                id: rowLayout
                visible: model.showAuthor && !isEmote
                Layout.fillWidth: true
                Layout.leftMargin: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing : 0
                Layout.rightMargin: Kirigami.Units.largeSpacing
                Layout.preferredWidth: nameLabel.implicitWidth + timeLabel.implicitWidth + Kirigami.Units.largeSpacing * 2
                Layout.maximumWidth: bubbleMaxWidth
                implicitHeight: visible ? nameLabel.implicitHeight : 0

                QQC2.Label {
                    id: nameLabel
                    topInset: 0

                    visible: model.showAuthor && !isEmote
                    anchors.left: rowLayout.left
                    anchors.right: timeLabel.left
                    anchors.rightMargin: Kirigami.Units.smallSpacing

                    text: visible ? author.displayName : ""
                    font.weight: Font.Bold
                    color: author.color
                    wrapMode: Text.Wrap
                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            userDetailDialog.createObject(QQC2.ApplicationWindow.overlay, {
                                room: currentRoom,
                                user: author.object,
                                displayName: author.displayName,
                                avatarMediaId: author.avatarMediaId,
                                avatarUrl: author.avatarUrl
                            }).open();
                        }
                    }
                }
                QQC2.Label {
                    id: timeLabel
                    anchors.right: rowLayout.right
                    visible: model.showAuthor && !isEmote
                    text: visible ? time.toLocaleTimeString(Locale.ShortFormat) : ""
                    color: Kirigami.Theme.disabledTextColor
                }
            }
            Loader {
                id: replyLoader
                active: model.reply !== undefined
                source: 'qrc:imports/NeoChat/Component/Timeline/ReplyComponent.qml'
                visible: active
                Layout.topMargin: Kirigami.Units.smallSpacing
                Layout.bottomMargin: Config.compactLayout ? 0 : Kirigami.Units.smallSpacing

                Connections {
                    target: replyLoader.item
                    function onClicked() {
                        replyClicked(reply.eventId)
                    }
                }
            }
        }

        background: Kirigami.ShadowedRectangle {
            visible: cardBackground && !Config.compactLayout
            color: model.isHighlighted ? Kirigami.Theme.positiveBackgroundColor : Kirigami.Theme.backgroundColor
            radius: Kirigami.Units.smallSpacing
            shadow.size: Kirigami.Units.smallSpacing
            shadow.color: !model.isHighlighted ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
            border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
            border.width: Kirigami.Units.devicePixelRatio
        }
    }

    Loader {
        id: loader
        anchors {
            left: bubble.left
            right: parent.right
            top: bubble.bottom
            topMargin: active && !Config.compactLayout ? Kirigami.Units.smallSpacing : 0
        }
        height: active ? item.implicitHeight : 0
        //Layout.bottomMargin: readMarker ? Kirigami.Units.smallSpacing : 0
        active: eventType !== "state" && eventType !== "notice" && reaction != undefined && reaction.length > 0
        visible: active
        sourceComponent: ReactionDelegate { }
    }
}
