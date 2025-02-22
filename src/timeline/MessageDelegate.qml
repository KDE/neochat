// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat

/**
 * @brief The base delegate for all messages in the timeline.
 *
 * This supports a message bubble plus sender avatar for each message
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
TimelineDelegate {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The index of the delegate in the model.
     */
    required property var index

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The message author.
     *
     * A Quotient::RoomMember object.
     *
     * @sa Quotient::RoomMember
     */
    required property NeochatRoomMember author

    /**
     * @brief Whether the message author should be shown.
     */
    required property bool showAuthor

    /**
     * @brief The model to visualise the content of the message.
     */
    required property MessageContentModel contentModel

    /**
     * @brief The date of the event as a string.
     */
    required property string section

    /**
     * @brief Whether the section header should be shown.
     */
    required property bool showSection

    /**
     * @brief A model with the first 5 other user read markers for this message.
     */
    required property var readMarkers

    /**
     * @brief Whether the other user read marker component should be shown.
     */
    required property bool showReadMarkers

    /**
     * @brief Whether the message in a thread.
     */
    required property bool isThreaded

    /**
     * @brief The Matrix ID of the root message in the thread, if any.
     */
    required property string threadRoot

    /**
     * @brief Whether this message has a local user mention.
     */
    required property bool isHighlighted

    /**
     * @brief Whether an event is waiting to be accepted by the server.
     */
    required property bool isPending

    /**
     * @brief Whether the event can be edited by the local user.
     */
    required property bool isEditable

    /**
     * @brief Whether an encrypted message is sent in a verified session.
     */
    required property bool verified

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
     * @brief Open the any message media externally.
     */
    signal openExternally

    /**
     * @brief The reply has been clicked.
     */
    signal replyClicked(string eventID)
    onReplyClicked: eventID => ListView.view.goToEvent(eventID)

    /**
     * @brief The main delegate content item to show in the bubble.
     */
    property var bubbleContent

    /**
     * @brief Whether the bubble background is enabled.
     */
    property bool cardBackground: true

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

    /**
     * @brief The user selected text.
     */
    property string selectedText: ""

    /**
     * @brief The user hovered link.
     */
    property string hoveredLink: ""

    onIsTemporaryHighlightedChanged: if (isTemporaryHighlighted) {
        temporaryHighlightTimer.start();
    }

    Timer {
        id: temporaryHighlightTimer

        interval: 1500
        onTriggered: isTemporaryHighlighted = false
    }

    /**
     * @brief The width available to the bubble content.
     */
    property real contentMaxWidth: (root.isThreaded ? bubbleSizeHelper.parentWidth : bubbleSizeHelper.currentWidth) - bubble.leftPadding - bubble.rightPadding

    width: parent?.width
    rightPadding: NeoChatConfig.compactLayout && root.ListView.view.width >= Kirigami.Units.gridUnit * 20 ? Kirigami.Units.gridUnit * 2 + Kirigami.Units.largeSpacing : Kirigami.Units.largeSpacing

    alwaysFillWidth: NeoChatConfig.compactLayout

    contentItem: ColumnLayout {
        spacing: Kirigami.Units.smallSpacing

        SectionDelegate {
            id: sectionDelegate
            Layout.fillWidth: true
            visible: root.showSection
            labelText: root.section
            colorSet: NeoChatConfig.compactLayout || root.alwaysFillWidth ? Kirigami.Theme.View : Kirigami.Theme.Window
        }
        QQC2.ItemDelegate {
            id: mainContainer

            Layout.fillWidth: true
            Layout.topMargin: root.showAuthor ? Kirigami.Units.largeSpacing : (NeoChatConfig.compactLayout ? 1 : Kirigami.Units.smallSpacing)
            Layout.leftMargin: Kirigami.Units.smallSpacing
            Layout.rightMargin: Kirigami.Units.smallSpacing

            implicitHeight: Math.max(root.showAuthor ? avatar.implicitHeight : 0, bubble.height)

            // show hover actions
            onHoveredChanged: {
                if (hovered && !Kirigami.Settings.isMobile) {
                    root.setHoverActionsToDelegate();
                }
            }

            KirigamiComponents.AvatarButton {
                id: avatar
                width: visible || NeoChatConfig.showAvatarInTimeline ? Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2 : 0
                height: width
                anchors {
                    left: parent.left
                    leftMargin: Kirigami.Units.smallSpacing
                    top: parent.top
                    topMargin: Kirigami.Units.smallSpacing
                }

                visible: ((root.showAuthor ?? false) || root.isThreaded) && NeoChatConfig.showAvatarInTimeline && (NeoChatConfig.compactLayout || !_private.showUserMessageOnRight)
                name: root.author.displayName
                source: root.author.avatarUrl
                color: root.author.color
                asynchronous: true
                QQC2.ToolTip.text: root.author.htmlSafeDisambiguatedName

                onClicked: RoomManager.resolveResource(root.author.uri)
            }
            Bubble {
                id: bubble
                anchors.left: avatar.right
                anchors.leftMargin: Kirigami.Units.largeSpacing
                anchors.rightMargin: rightPadding
                maxContentWidth: root.contentMaxWidth

                topPadding: NeoChatConfig.compactLayout ? Kirigami.Units.smallSpacing / 2 : Kirigami.Units.largeSpacing
                bottomPadding: NeoChatConfig.compactLayout ? Kirigami.Units.mediumSpacing / 2 : Kirigami.Units.largeSpacing
                leftPadding: NeoChatConfig.compactLayout ? 0 : Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
                rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

                state: _private.showUserMessageOnRight ? "userMessageOnRight" : "userMessageOnLeft"
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
                            anchors.right: root.isThreaded ? parent.right : undefined
                        }
                    }
                ]

                room: root.room
                index: root.index
                author: root.author
                showAuthor: root.showAuthor
                isThreaded: root.isThreaded

                contentModel: root.contentModel
                timeline: root.ListView.view

                showHighlight: root.showHighlight

                isPending: root.isPending

                onReplyClicked: eventId => {
                    root.replyClicked(eventId);
                }
                onSelectedTextChanged: selectedText => {
                    root.selectedText = selectedText;
                }
                onHoveredLinkChanged: hoveredLink => {
                    root.hoveredLink = hoveredLink;
                }
                onShowMessageMenu: _private.showMessageMenu()

                showBackground: root.cardBackground && !NeoChatConfig.compactLayout
            }

            background: Rectangle {
                visible: mainContainer.hovered && (NeoChatConfig.compactLayout || root.alwaysFillWidth)
                color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15)
                radius: Kirigami.Units.cornerRadius
            }

            TapHandler {
                acceptedButtons: Qt.RightButton
                onTapped: _private.showMessageMenu()
            }

            TapHandler {
                acceptedDevices: PointerDevice.TouchScreen
                acceptedButtons: Qt.LeftButton
                onLongPressed: _private.showMessageMenu()
            }
        }
        AvatarFlow {
            Layout.alignment: Qt.AlignRight
            Layout.rightMargin: Kirigami.Units.largeSpacing
            visible: root.showReadMarkers
            model: root.readMarkers
        }

        DelegateSizeHelper {
            id: bubbleSizeHelper
            startBreakpoint: Kirigami.Units.gridUnit * 25
            endBreakpoint: Kirigami.Units.gridUnit * 40
            startPercentWidth: root.alwaysFillWidth ? 100 : 90
            endPercentWidth: root.alwaysFillWidth ? 100 : 60

            parentWidth: mainContainer.availableWidth - (NeoChatConfig.showAvatarInTimeline ? avatar.width + bubble.anchors.leftMargin : 0)
        }
    }

    function isVisibleInTimeline() {
        let yoff = Math.round(y - ListView.view.contentY);
        return (yoff + height > 0 && yoff < ListView.view.height);
    }

    function setHoverActionsToDelegate() {
        if (ListView.view.setHoverActionsToDelegate) {
            ListView.view.setHoverActionsToDelegate(root);
        }
    }

    QtObject {
        id: _private

        /**
         * @brief Whether local user messages should be aligned right.
         */
        property bool showUserMessageOnRight: NeoChatConfig.showLocalMessagesOnRight && root.author.isLocalMember && !NeoChatConfig.compactLayout && !root.alwaysFillWidth && !root.isThreaded

        function showMessageMenu() {
            RoomManager.viewEventMenu(root.eventId, root.room, root.author, root.selectedText, root.hoveredLink);
        }
    }
}
