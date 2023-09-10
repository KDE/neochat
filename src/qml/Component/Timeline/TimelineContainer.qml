// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents

import org.kde.neochat 1.0

/**
 * @brief The base delegate for all messages in the timeline.
 *
 * This supports a message bubble plus sender avatar for each message as well as reactions
 * and read markers. A date section can be show for when the message is on a different
 * day to the previous one.
 *
 * The component is designed for all messages, positioning them in the timeline with
 * variable padding depending on the window width. Local user messages are highlighted
 * and can also be aligned to the right if configured.
 *
 * This component also supports a compact mode where the padding is adjusted, the
 * background is hidden and the delegate spans the full width of the timeline.
 */
ColumnLayout {
    id: root

    /**
     * @brief The index of the delegate in the model.
     */
    required property var index

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The timestamp of the message.
     */
    required property var time

    /**
     * @brief The timestamp of the message as a string.
     */
    required property string timeString

    /**
     * @brief The message author.
     *
     * This should consist of the following:
     *  - id - The matrix ID of the author.
     *  - isLocalUser - Whether the author is the local user.
     *  - avatarSource - The mxc URL for the author's avatar in the current room.
     *  - avatarMediaId - The media ID of the author's avatar.
     *  - avatarUrl - The mxc URL for the author's avatar.
     *  - displayName - The display name of the author.
     *  - display - The name of the author.
     *  - color - The color for the author.
     *  - object - The Quotient::User object for the author.
     *
     * @sa Quotient::User
     */
    required property var author

    /**
     * @brief Whether the author should be shown.
     */
    required property bool showAuthor

    /**
     * @brief Whether the author should always be shown.
     *
     * This is primarily used when these delegates are used in a filtered list of
     * events rather than a sequential timeline, e.g. the media model view.
     *
     * @note This setting still respects the avatar configuration settings.
     */
    property bool alwaysShowAuthor: false

    /**
     * @brief The delegate type of the message.
     */
    required property int delegateType

    /**
     * @brief The display text of the message.
     */
    required property string display

    /**
     * @brief The display text of the message as plain text.
     */
    required property string plainText

    /**
     * @brief The date of the event as a string.
     */
    required property string section

    /**
     * @brief Whether the section header should be shown.
     */
    required property bool showSection

    /**
     * @brief A model with the reactions to the message in.
     */
    required property var reaction

    /**
     * @brief Whether the reaction component should be shown.
     */
    required property bool showReactions

    /**
     * @brief A model with the first 5 other user read markers for this message.
     */
    required property var readMarkers

    /**
     * @brief String with the display name and matrix ID of the other user read markers.
     */
    required property string readMarkersString

    /**
     * @brief The number of other users at the event after the first 5.
     */
    required property var excessReadMarkers

    /**
     * @brief Whether the other user read marker component should be shown.
     */
    required property bool showReadMarkers

    /**
     * @brief The matrix ID of the reply event.
     */
    required property var replyId

    /**
     * @brief The reply author.
     *
     * This should consist of the following:
     *  - id - The matrix ID of the reply author.
     *  - isLocalUser - Whether the reply author is the local user.
     *  - avatarSource - The mxc URL for the reply author's avatar in the current room.
     *  - avatarMediaId - The media ID of the reply author's avatar.
     *  - avatarUrl - The mxc URL for the reply author's avatar.
     *  - displayName - The display name of the reply author.
     *  - display - The name of the reply author.
     *  - color - The color for the reply author.
     *  - object - The Quotient::User object for the reply author.
     *
     * @sa Quotient::User
     */
    required property var replyAuthor

    /**
     * @brief The delegate type of the message replied to.
     */
    required property int replyDelegateType

    /**
     * @brief The display text of the message replied to.
     */
    required property string replyDisplay

    /**
     * @brief The media info for the reply event.
     *
     * This could be an image, audio, video or file.
     *
     * This should consist of the following:
     *  - source - The mxc URL for the media.
     *  - mimeType - The MIME type of the media.
     *  - mimeIcon - The MIME icon name.
     *  - size - The file size in bytes.
     *  - duration - The length in seconds of the audio media (audio/video only).
     *  - width - The width in pixels of the audio media (image/video only).
     *  - height - The height in pixels of the audio media (image/video only).
     *  - tempInfo - mediaInfo (with the same properties as this except no tempInfo) for a temporary image while the file downloads (image/video only).
     */
    required property var replyMediaInfo

    /**
     * @brief Whether this message is replying to another.
     */
    required property bool isReply

    /**
     * @brief Whether this message has a local user mention.
     */
    required property bool isHighlighted

    /**
     * @brief Whether an event is waiting to be accepted by the server.
     */
    required property bool isPending

    /**
     * @brief Progress info when downloading files.
     *
     * @sa Quotient::FileTransferInfo
     */
    required property var progressInfo

    /**
     * @brief Whether an encrypted message is sent in a verified session.
     */
    required property bool verified

    /**
     * @brief The full message source JSON.
     */
    required property var jsonSource

    /**
     * @brief The x position of the message bubble.
     *
     * @note Used for positioning the hover actions.
     */
    readonly property real bubbleX: bubble.x + bubble.anchors.leftMargin

    /**
     * @brief The y position of the message bubble.
     *
     * @note Used for positioning the hover actions.
     */
    readonly property alias bubbleY: mainContainer.y

    /**
     * @brief The width of the message bubble.
     *
     * @note Used for sizing the hover actions.
     */
    readonly property alias bubbleWidth: bubble.width

    /**
     * @brief Whether this message is hovered.
     */
    readonly property alias hovered: bubble.hovered

    /**
     * @brief Open the context menu for the message.
     */
    signal openContextMenu

    /**
     * @brief Open the any message media externally.
     */
    signal openExternally()

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked(string eventID)

    onReplyClicked: eventID => ListView.view.goToEvent(eventID)

    /**
     * @brief The component to display the delegate type.
     *
     * This is used by the inherited delegates to assign a component to visualise
     * the message content for that delegate type.
     */
    default property alias innerObject : column.children

    /**
     * @brief Whether the bubble background is enabled.
     */
    property bool cardBackground: true

    /**
     * @brief Whether the delegate should always stretch to the maximum availabel width.
     */
    property bool alwaysMaxWidth: false

    /**
     * @brief Whether local user messages should be aligned right.
     *
     * TODO: make private
     */
    property bool showUserMessageOnRight: Config.showLocalMessagesOnRight && root.author.isLocalUser && !Config.compactLayout && !alwaysMaxWidth

    /**
     * @brief Whether the message should be highlighted.
     */
    property bool showHighlight: root.isHighlighted || isTemporaryHighlighted

    /**
     * @brief Whether the message should temporarily be highlighted.
     *
     * Normally triggered when jumping to the event in the timeline, e.g. when a reply
     * is clicked.
     */
    property bool isTemporaryHighlighted: false

    onIsTemporaryHighlightedChanged: if (isTemporaryHighlighted) temporaryHighlightTimer.start()

    Timer {
        id: temporaryHighlightTimer

        interval: 1500
        onTriggered: isTemporaryHighlighted = false
    }

    readonly property int contentMaxWidth: bubbleSizeHelper.currentWidth

    width: parent ? timelineDelegateSizeHelper.currentWidth : 0
    spacing: Kirigami.Units.smallSpacing

    state: Config.compactLayout || root.alwaysMaxWidth ? "alignLeft" : "alignCenter"
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
        // Fill ListView width without affecting size helper and the rest of components too much
        Layout.rightMargin: root.parent ? root.width - root.parent.width : 0
        visible: root.showSection
        labelText: root.section
        colorSet: Config.compactLayout || root.alwaysMaxWidth ? Kirigami.Theme.View : Kirigami.Theme.Window
    }

    QQC2.ItemDelegate {
        id: mainContainer

        Layout.fillWidth: true
        Layout.topMargin: root.showAuthor || root.alwaysShowAuthor ? Kirigami.Units.largeSpacing : (Config.compactLayout ? 1 : Kirigami.Units.smallSpacing)
        Layout.leftMargin: Kirigami.Units.smallSpacing
        Layout.rightMargin: Kirigami.Units.smallSpacing

        implicitHeight: Math.max(root.showAuthor || root.alwaysShowAuthor ? avatar.implicitHeight : 0, bubble.height)

        Component.onCompleted: {
            if (root.isReply && root.replyDelegateType === DelegateType.Other) {
                currentRoom.loadReply(root.eventId, root.replyId)
            }
        }

        // show hover actions
        onHoveredChanged: {
            if (hovered && !Kirigami.Settings.isMobile) {
                root.setHoverActionsToDelegate()
            }
        }

        KirigamiComponents.Avatar {
            id: avatar
            width: visible || Config.showAvatarInTimeline ? Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2: 0
            height: width
            anchors {
                left: parent.left
                leftMargin: Kirigami.Units.smallSpacing
                top: parent.top
                topMargin: Kirigami.Units.smallSpacing
            }

            visible: (root.showAuthor || root.alwaysShowAuthor) &&
                Config.showAvatarInTimeline &&
                (Config.compactLayout || !showUserMessageOnRight)
            name: root.author.displayName
            source: root.author.avatarSource
            color: root.author.color

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    RoomManager.visitUser(root.author.object, "mention")
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
                left: avatar.right
                leftMargin: Kirigami.Units.largeSpacing
                rightMargin: Kirigami.Units.largeSpacing
            }
            // HACK: anchoring didn't reset anchors.right when switching from parent.right to undefined reliably
            width: Config.compactLayout || root.alwaysMaxWidth ? mainContainer.width - (Config.showAvatarInTimeline ? Kirigami.Units.gridUnit * 2 : 0) + Kirigami.Units.largeSpacing * 2 : implicitWidth

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
                    visible: root.isPending && Config.showLocalMessagesOnRight
                }
                ColumnLayout {
                    id: column
                    spacing: Kirigami.Units.smallSpacing
                    RowLayout {
                        id: rowLayout

                        spacing: Kirigami.Units.smallSpacing
                        visible: root.showAuthor || root.alwaysShowAuthor

                        QQC2.Label {
                            id: nameLabel

                            Layout.maximumWidth: contentMaxWidth - timeLabel.implicitWidth - rowLayout.spacing

                            text: visible ? root.author.displayName : ""
                            textFormat: Text.PlainText
                            font.weight: Font.Bold
                            color: root.author.color
                            elide: Text.ElideRight
                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: {
                                    RoomManager.visitUser(root.author.object, "mention")
                                }
                            }
                        }
                        QQC2.Label {
                            id: timeLabel

                            text: root.timeString
                            color: Kirigami.Theme.disabledTextColor
                            QQC2.ToolTip.visible: hoverHandler.hovered
                            QQC2.ToolTip.text: root.time.toLocaleString(Qt.locale(), Locale.LongFormat)
                            QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

                            HoverHandler {
                                id: hoverHandler
                            }
                        }
                    }
                    Loader {
                        id: replyLoader

                        Layout.maximumWidth: contentMaxWidth

                        active: root.isReply && root.replyDelegateType !== DelegateType.Other
                        visible: active

                        sourceComponent: ReplyComponent {
                            author: root.replyAuthor
                            type: root.replyDelegateType
                            display: root.replyDisplay
                            mediaInfo: root.replyMediaInfo
                            contentMaxWidth: bubbleSizeHelper.currentWidth
                        }

                        Connections {
                            target: replyLoader.item
                            function onReplyClicked() {
                                replyClicked(root.replyId)
                            }
                        }
                    }
                }
                Kirigami.Icon {
                    source: "content-loading-symbolic"
                    width: height
                    Layout.preferredWidth: Kirigami.Units.iconSizes.small
                    Layout.preferredHeight: Kirigami.Units.iconSizes.small
                    visible: root.isPending && !Config.showLocalMessagesOnRight
                }
            }

            background: Item {
                Kirigami.ShadowedRectangle {
                    id: bubbleBackground
                    visible: cardBackground && !Config.compactLayout
                    anchors.fill: parent
                    Kirigami.Theme.colorSet: Kirigami.Theme.View
                    Kirigami.Theme.inherit: false
                    color: if (root.author.isLocalUser) {
                        return Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15)
                    } else if (root.showHighlight) {
                        return Kirigami.Theme.positiveBackgroundColor
                    } else {
                        return Kirigami.Theme.backgroundColor
                    }
                    radius: Kirigami.Units.smallSpacing
                    shadow {
                        size:  Kirigami.Units.smallSpacing
                        color: root.isHighlighted ? Qt.rgba(0.0, 0.0, 0.0, 0.10) : Qt.rgba(Kirigami.Theme.textColor.r, Kirigami.Theme.textColor.g, Kirigami.Theme.textColor.b, 0.10)
                    }

                    Behavior on color {
                        ColorAnimation { duration: Kirigami.Units.shortDuration }
                    }
                }
            }
        }

        background: Rectangle {
            visible: mainContainer.hovered && (Config.compactLayout || root.alwaysMaxWidth)
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
        Layout.maximumWidth: root.width - Kirigami.Units.largeSpacing * 2
        Layout.alignment: showUserMessageOnRight ? Qt.AlignRight : Qt.AlignLeft
        Layout.leftMargin: showUserMessageOnRight ? 0 : bubble.x + bubble.anchors.leftMargin
        Layout.rightMargin: showUserMessageOnRight ? Kirigami.Units.largeSpacing : 0

        visible: root.showReactions
        model: root.reaction

        onReactionClicked: (reaction) => currentRoom.toggleReaction(root.eventId, reaction)
    }
    AvatarFlow {
        Layout.alignment: Qt.AlignRight
        Layout.rightMargin: Kirigami.Units.largeSpacing
        visible: root.showReadMarkers
        model: root.readMarkers
        toolTipText: root.readMarkersString
        excessAvatars: root.excessReadMarkers
    }

    function isVisibleInTimeline() {
        let yoff = Math.round(y - ListView.view.contentY);
        return (yoff + height > 0 && yoff < ListView.view.height)
    }

    Component {
        id: messageDelegateContextMenu
        MessageDelegateContextMenu {}
    }

    Component {
        id: fileDelegateContextMenu
        FileDelegateContextMenu {}
    }

    /// Open message context dialog for file and videos
    function openFileContext(file) {
        const contextMenu = fileDelegateContextMenu.createObject(root, {
            author: root.author,
            eventId: root.eventId,
            source: root.jsonSource,
            file: file,
            progressInfo: root.progressInfo,
            plainText: root.plainText,
        });
        contextMenu.open();
    }

    /// Open context menu for normal message
    function openMessageContext(selectedText) {
        const contextMenu = messageDelegateContextMenu.createObject(root, {
            selectedText: selectedText,
            author: root.author,
            eventId: root.eventId,
            eventSource: root.jsonSource,
            eventType: root.delegateType,
            plainText: root.plainText,
            htmlText: root.display,
        });
        contextMenu.open();
    }

    function setHoverActionsToDelegate() {
        if (ListView.view.setHoverActionsToDelegate) {
            ListView.view.setHoverActionsToDelegate(root)
        }
    }

    DelegateSizeHelper {
        id: timelineDelegateSizeHelper
        startBreakpoint: Kirigami.Units.gridUnit * 46
        endBreakpoint: Kirigami.Units.gridUnit * 66
        startPercentWidth: 100
        endPercentWidth: Config.compactLayout || root.alwaysMaxWidth ? 100 : 85
        maxWidth: Config.compactLayout || root.alwaysMaxWidth ? -1 : Kirigami.Units.gridUnit * 60

        parentWidth: root.parent ? root.parent.width - (Config.compactLayout && root.ListView.view.width >= Kirigami.Units.gridUnit * 20 ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing : 0) : 0
    }
    DelegateSizeHelper {
        id: bubbleSizeHelper
        startBreakpoint: Kirigami.Units.gridUnit * 25
        endBreakpoint: Kirigami.Units.gridUnit * 40
        startPercentWidth: Config.compactLayout || root.alwaysMaxWidth ? 100 : 90
        endPercentWidth: Config.compactLayout || root.alwaysMaxWidth ? 100 : 60

        parentWidth: mainContainer.availableWidth - (Config.showAvatarInTimeline ? avatar.width + bubble.anchors.leftMargin : 0)
    }
}
