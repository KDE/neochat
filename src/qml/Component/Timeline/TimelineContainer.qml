// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

ColumnLayout {
    id: root

    signal openContextMenu
    signal openExternally()
    signal replyClicked(string eventID)

    onReplyClicked: ListView.view.goToEvent(eventID)

    default property alias innerObject : column.children

    property Item hoverComponent: hoverActions ?? null
    property bool cardBackground: true
    property bool showUserMessageOnRight: Config.showLocalMessagesOnRight && model.author.isLocalUser && !Config.compactLayout
    property bool isHighlighted: model.isHighlighted || isTemporaryHighlighted
    property bool isTemporaryHighlighted: false

    onIsTemporaryHighlightedChanged: if (isTemporaryHighlighted) temporaryHighlightTimer.start()

    Timer {
        id: temporaryHighlightTimer

        interval: 1500
        onTriggered: isTemporaryHighlighted = false
    }

    // The bubble and delegate widths are allowed to grow once the ListView gets beyond a certain size
    // extraWidth defines this as the excess after a certain ListView width, capped to a max value
    readonly property int extraWidth: messageListView.width >= Kirigami.Units.gridUnit * 46 ? Math.min((messageListView.width - Kirigami.Units.gridUnit * 46), Kirigami.Units.gridUnit * 20) : 0
    readonly property int bubbleMaxWidth: Kirigami.Units.gridUnit * 20 + extraWidth * 0.5
    readonly property int delegateWidth: Config.compactLayout ? messageListView.width - Kirigami.Units.smallSpacing - (ListView.view.width >= Kirigami.Units.gridUnit * 20 ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing : 0) : Math.min(messageListView.width, Kirigami.Units.gridUnit * 40 + extraWidth)
    readonly property int contentMaxWidth: Config.compactLayout ? width - (Config.showAvatarInTimeline ? Kirigami.Units.gridUnit * 2 : 0) - Kirigami.Units.largeSpacing * 4 : Math.min(width - Kirigami.Units.gridUnit * 2 - Kirigami.Units.largeSpacing * 6, bubbleMaxWidth)

    width: delegateWidth
    spacing: Kirigami.Units.smallSpacing

    state: Config.compactLayout ? "alignLeft" : "alignCenter"
    // Align left when in compact mode and center when using bubbles
    states: [
        State {
            name: "alignLeft"
            AnchorChanges {
                target: root
                anchors.horizontalCenter: undefined
                anchors.left: parent ? parent.left : undefined
            }
        },
        State {
            name: "alignCenter"
            AnchorChanges {
                target: root
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

        Layout.fillWidth: true
        visible: model.showSection
        labelText: model.showSection ? section : ""
    }

    QQC2.ItemDelegate {
        id: mainContainer

        Layout.fillWidth: true
        Layout.topMargin: showAuthor ? Kirigami.Units.largeSpacing : (Config.compactLayout ? 1 : Kirigami.Units.smallSpacing)
        Layout.leftMargin: Kirigami.Units.smallSpacing
        Layout.rightMargin: Kirigami.Units.smallSpacing

        implicitHeight: Math.max(model.showAuthor ? avatar.implicitHeight : 0, bubble.height)

        Component.onCompleted: {
            if (model.isReply && model.reply === undefined) {
                messageEventModel.loadReply(sortedMessageEventModel.mapToSource(collapseStateProxyModel.mapToSource(collapseStateProxyModel.index(model.index, 0))))
            }
        }

        // show hover actions
        onHoveredChanged: {
            if (hovered && !Kirigami.Settings.isMobile) {
                updateHoverComponent();
            }
        }


        // Show hover actions by updating the global hover component to this delegate
        function updateHoverComponent() {
            if (!hoverComponent) {
                return;
            }
            if (hovered && !Kirigami.Settings.isMobile) {
                hoverComponent.delegate = root
                hoverComponent.bubble = bubble
                hoverComponent.event = model
                hoverComponent.updateFunction = updateHoverComponent;
            }
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
            anchors {
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
            leftPadding: Config.compactLayout ? 0 : Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
            hoverEnabled: true

            anchors {
                leftMargin: Kirigami.Units.smallSpacing
                rightMargin: Kirigami.Units.largeSpacing
            }
            // HACK: anchoring didn't reset anchors.right when switching from parent.right to undefined reliably
            width: Config.compactLayout ? mainContainer.width - (Config.showAvatarInTimeline ? Kirigami.Units.gridUnit * 2 : 0) + Kirigami.Units.largeSpacing * 2 : implicitWidth

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

            contentItem: RowLayout {
                Kirigami.Icon {
                    source: "content-loading-symbolic"
                    width: height
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    visible: model.isPending && Config.showLocalMessagesOnRight
                }
                ColumnLayout {
                    id: column
                    spacing: Kirigami.Units.smallSpacing
                    RowLayout {
                        id: rowLayout

                        spacing: Kirigami.Units.smallSpacing
                        visible: model.showAuthor

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

                            text: visible ? model.time.toLocaleTimeString(Qt.locale(), Locale.ShortFormat) : ""
                            color: Kirigami.Theme.disabledTextColor
                            QQC2.ToolTip.visible: hoverHandler.hovered
                            QQC2.ToolTip.text: model.time.toLocaleString(Qt.locale(), Locale.LongFormat)
                            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                            HoverHandler {
                                id: hoverHandler
                            }
                        }
                    }
                    Loader {
                        id: replyLoader

                        Layout.maximumWidth: contentMaxWidth

                        active: model.reply !== undefined
                        visible: active

                        sourceComponent: ReplyComponent {
                            name: currentRoom.htmlSafeMemberName(reply.author.id)
                            avatar: reply.author.avatarMediaId ? ("image://mxc/" + reply.author.avatarMediaId) : ""
                            color: reply.author.color
                        }

                        Connections {
                            target: replyLoader.item
                            function onReplyClicked() {
                                replyClicked(reply.eventId)
                            }
                        }
                    }
                }
                Kirigami.Icon {
                    source: "content-loading-symbolic"
                    width: height
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    visible: model.isPending && !Config.showLocalMessagesOnRight
                }
            }


            background: Item {
                Kirigami.ShadowedRectangle {
                    id: bubbleBackground
                    visible: cardBackground && !Config.compactLayout
                    anchors.fill: parent
                    Kirigami.Theme.colorSet: Kirigami.Theme.View
                    color: {
                        if (model.author.isLocalUser) {
                            return Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15)
                        } else if (root.isHighlighted) {
                            return Kirigami.Theme.positiveBackgroundColor
                        } else {
                            return Kirigami.Theme.backgroundColor
                        }
                    }
                    radius: Kirigami.Units.smallSpacing
                    shadow.size: Kirigami.Units.smallSpacing
                    shadow.color: root.isHighlighted ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                    border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                    border.width: 1

                    Behavior on color {
                        enabled: isTemporaryHighlighted
                        ColorAnimation {target: bubbleBackground; duration: Kirigami.Units.veryLongDuration; easing.type: Easing.InOutCubic}
                    }
                }
            }
        }

        background: Rectangle {
            visible: mainContainer.hovered
            color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15)
            radius: Kirigami.Units.smallSpacing
        }

        TapHandler {
            acceptedDevices: PointerDevice.Mouse
            acceptedButtons: Qt.RightButton
            onTapped: root.openContextMenu()
        }

        TapHandler {
            acceptedButtons: Qt.LeftButton
            onLongPressed: root.openContextMenu()
        }
    }

    ReactionDelegate {
        Layout.maximumWidth: delegateWidth - Kirigami.Units.largeSpacing * 2
        Layout.alignment: showUserMessageOnRight ? Qt.AlignRight : Qt.AlignLeft
        Layout.leftMargin: showUserMessageOnRight ? 0 : bubble.x + bubble.anchors.leftMargin
        Layout.rightMargin: showUserMessageOnRight ? Kirigami.Units.largeSpacing : 0

        visible: delegateType !== MessageEventModel.State && delegateType !== MessageEventModel.Notice && reaction != undefined && reaction.length > 0
    }
    AvatarFlow {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: Kirigami.Units.largeSpacing
        visible: showReadMarkers
        model: readMarkers
        toolTipText: readMarkersString
    }

    function isVisibleInTimeline() {
        let yoff = Math.round(y - ListView.view.contentY);
        return (yoff + height > 0 && yoff < ListView.view.height)
    }
}
