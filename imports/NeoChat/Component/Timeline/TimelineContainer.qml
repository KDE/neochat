// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0
import NeoChat.Dialog 1.0

QQC2.ItemDelegate {
    id: timelineContainer
    default property alias innerObject : column.children
    // readonly property bool failed: marks == EventStatus.SendingFailed

    property bool isEmote: false
    property bool cardBackground: true
    property bool isHighlighted: model.isHighlighted || isTemporaryHighlighted
    property bool isTemporaryHighlighted: false

    onIsTemporaryHighlightedChanged: if (isTemporaryHighlighted) temporaryHighlightTimer.start()

    Timer {
        id: temporaryHighlightTimer

        interval: 1500
        onTriggered: isTemporaryHighlighted = false
    }

    signal openContextMenu

    // The bubble and delegate widths are allowed to grow once the ListView gets beyond a certain size
    // extraWidth defines this as the excess after a certain ListView width, capped to a max value
    readonly property int extraWidth: messageListView.width >= Kirigami.Units.gridUnit * 46 ? Math.min((messageListView.width - Kirigami.Units.gridUnit * 46), Kirigami.Units.gridUnit * 20) : 0
    readonly property int bubbleMaxWidth: Kirigami.Units.gridUnit * 20 + extraWidth * 0.5
    readonly property int delegateMaxWidth: Config.compactLayout ? messageListView.width : Math.min(messageListView.width, Kirigami.Units.gridUnit * 40 + extraWidth)
    readonly property int contentMaxWidth: Config.compactLayout ? width - (Config.showAvatarInTimeline ? Kirigami.Units.gridUnit * 2 : 0) - Kirigami.Units.largeSpacing * 4 : Math.min(width - Kirigami.Units.gridUnit * 2 - Kirigami.Units.largeSpacing * 6, bubbleMaxWidth)

    property bool showUserMessageOnRight: Config.showLocalMessagesOnRight &&
        model.author.isLocalUser && !Config.compactLayout

    signal openExternally()
    signal replyClicked(string eventID)

    Component.onCompleted: {
        if (model.isReply && model.reply === undefined) {
            messageEventModel.loadReply(sortedMessageEventModel.mapToSource(sortedMessageEventModel.index(model.index, 0)))
        }
    }

    topPadding: 0
    bottomPadding: 0
    topInset: showAuthor ? Kirigami.Units.largeSpacing : (Config.compactLayout ? 1 : Kirigami.Units.smallSpacing)
    leftInset: Kirigami.Units.smallSpacing
    rightInset: Kirigami.Units.smallSpacing
    width: delegateMaxWidth
    height: sectionDelegate.height + Math.max(model.showAuthor ? avatar.height : 0, bubble.implicitHeight) + loader.height + (showAuthor ? Kirigami.Units.largeSpacing : (Config.compactLayout ? 1 : Kirigami.Units.smallSpacing))
    background: Rectangle {
        visible: timelineContainer.hovered
        color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15)
        radius: Kirigami.Units.smallSpacing
    }

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
            hoverComponent.delegate = timelineContainer
            hoverComponent.bubble = bubble
            hoverComponent.updateFunction = updateHoverComponent;
            hoverComponent.event = model
        }
    }

    state: Config.compactLayout ? "alignLeft" : "alignCenter"
    // Align left when in compact mode and center when using bubbles
    states: [
        State {
            name: "alignLeft"
            AnchorChanges {
                target: timelineContainer
                anchors.horizontalCenter: undefined
                anchors.left: parent ? parent.left : undefined
            }
        },
        State {
            name: "alignCenter"
            AnchorChanges {
                target: timelineContainer
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
        width: visible || Config.showAvatarInTimeline ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.smallSpacing * 2 : 0
        height: width
        padding: Kirigami.Units.smallSpacing
        topInset: Kirigami.Units.smallSpacing
        bottomInset: Kirigami.Units.smallSpacing
        leftInset: Kirigami.Units.smallSpacing
        rightInset: Kirigami.Units.smallSpacing
        sourceSize.width: width
        sourceSize.height: width
        anchors {
            top: sectionDelegate.bottom
            topMargin: model.showAuthor ? Kirigami.Units.largeSpacing : (Config.compactLayout ? 1 : Kirigami.Units.smallSpacing)
            left: parent.left
            leftMargin: Kirigami.Units.smallSpacing
        }

        visible: model.showAuthor &&
            Config.showAvatarInTimeline &&
            (Config.compactLayout || !showUserMessageOnRight)
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
        topPadding: Config.compactLayout ? Kirigami.Units.smallSpacing / 2 : Kirigami.Units.largeSpacing
        bottomPadding: Config.compactLayout ? Kirigami.Units.mediumSpacing / 2 : Kirigami.Units.largeSpacing
        leftPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
        rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
        hoverEnabled: true

        anchors {
            top: avatar.top
            leftMargin: Kirigami.Units.smallSpacing
            rightMargin: showUserMessageOnRight ? Kirigami.Units.smallSpacing : Kirigami.Units.largeSpacing
        }
        // HACK: anchoring didn't reset anchors.right when switching from parent.right to undefined reliably
        width: Config.compactLayout ? timelineContainer.width - (Config.showAvatarInTimeline ? Kirigami.Units.gridUnit * 2 : 0) + Kirigami.Units.largeSpacing * 2 : implicitWidth

        state: showUserMessageOnRight ? "userMessageOnRight" : "userMessageOnLeft"
        // states for anchor animations on window resize
        // as setting anchors to undefined did not work reliably
        states: [
            State {
                name: "userMessageOnRight"
                AnchorChanges {
                    target: bubble
                    anchors.left: undefined
                    anchors.right: parent.right
                }
            },
            State {
                name: "userMessageOnLeft"
                AnchorChanges {
                    target: bubble
                    anchors.left: avatar.right
                    anchors.right: undefined
                }
            }
        ]

        transitions: [
            Transition {
                AnchorAnimation{duration: Kirigami.Units.longDuration; easing.type: Easing.OutCubic}
            }
        ]

        contentItem: ColumnLayout {
            id: column
            spacing: Kirigami.Units.smallSpacing
            RowLayout {
                id: rowLayout

                spacing: Kirigami.Units.smallSpacing
                visible: model.showAuthor && !isEmote

                QQC2.Label {
                    id: nameLabel

                    Layout.maximumWidth: contentMaxWidth - timeLabel.implicitWidth - rowLayout.spacing

                    text: visible ? author.displayName : ""
                    textFormat: Text.PlainText
                    font.weight: Font.Bold
                    color: author.color
                    elide: Text.ElideRight
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

                    text: visible ? time.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) : ""
                    color: Kirigami.Theme.disabledTextColor
                    QQC2.ToolTip.visible: hoverHandler.hovered
                    QQC2.ToolTip.text: time.toLocaleString(Qt.locale(), Locale.LongFormat)
                    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                    HoverHandler {
                        id: hoverHandler
                    }
                }
            }
            Loader {
                id: replyLoader
                active: model.reply !== undefined
                source: 'qrc:imports/NeoChat/Component/Timeline/ReplyComponent.qml'
                visible: active

                Connections {
                    target: replyLoader.item
                    function onClicked() {
                        replyClicked(reply.eventId)
                    }
                }
            }
        }

        background: Item {
            Kirigami.ShadowedRectangle {
                id: bubbleBackground
                visible: cardBackground && !Config.compactLayout
                anchors.fill: parent
                color: {
                    if (model.author.isLocalUser) {
                        return Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15)
                    } else if (timelineContainer.isHighlighted) {
                        return Kirigami.Theme.positiveBackgroundColor
                    } else {
                        return Kirigami.Theme.backgroundColor
                    }
                }
                radius: Kirigami.Units.smallSpacing
                shadow.size: Kirigami.Units.smallSpacing
                shadow.color: timelineContainer.isHighlighted ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                border.width: 1

                Behavior on color {
                    ColorAnimation {target: bubbleBackground; duration: Kirigami.Units.veryLongDuration; easing.type: Easing.InOutCubic}
                }
            }
        }
    }

    Loader {
        id: loader
        anchors {
            left: bubble.left
            right: parent.right
            top: bubble.bottom
            topMargin: active && Config.compactLayout ? 0 : Kirigami.Units.smallSpacing
        }
        height: active ? item.implicitHeight : 0
        //Layout.bottomMargin: readMarker ? Kirigami.Units.smallSpacing : 0
        active: eventType !== "state" && eventType !== "notice" && reaction != undefined && reaction.length > 0
        visible: active
        sourceComponent: ReactionDelegate { }
    }

    TapHandler {
        acceptedButtons: Qt.RightButton
        onTapped: timelineContainer.openContextMenu()
    }

    TapHandler {
        acceptedButtons: Qt.LeftButton
        onLongPressed: timelineContainer.openContextMenu()
    }
}
