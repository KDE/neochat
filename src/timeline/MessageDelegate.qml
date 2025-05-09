// SPDX-FileCopyrightText: 2020 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents

import org.kde.neochat
import org.kde.neochat.libneochat as LibNeoChat

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
MessageDelegateBase {
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
     * @brief The model to visualise the content of the message.
     */
    required property MessageContentModel contentModel

    /**
     * @brief The date of the event as a string.
     */
    required property string section

    /**
     * @brief A model with the first 5 other user read markers for this message.
     */
    required property var readMarkers

    /**
     * @brief The Matrix ID of the root message in the thread, if any.
     */
    required property string threadRoot

    /**
     * @brief Whether the message in a poll.
     */
    required property bool isPoll

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
     * @brief The y position of the message bubble.
     *
     * @note Used for positioning the hover actions.
     */
    readonly property alias bubbleY: bubble.y

    /**
     * @brief The width of the message bubble.
     *
     * @note Used for sizing the hover actions.
     */
    readonly property alias bubbleWidth: bubble.width

    /**
     * @brief Open the any message media externally.
     */
    signal openExternally

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

    Message.room: root.room
    Message.timeline: root.ListView.view
    Message.contentModel: root.contentModel
    Message.index: root.index
    Message.maxContentWidth: maxContentWidth - bubble.leftPadding - bubble.rightPadding

    width: parent?.width

    enableAvatars: NeoChatConfig?.showAvatarInTimeline ?? false
    compactMode: NeoChatConfig?.compactLayout ?? false
    showLocalMessagesOnRight: NeoChatConfig.showLocalMessagesOnRight

    contentItem: Bubble {
        id: bubble
        topPadding: NeoChatConfig.compactLayout ? Kirigami.Units.smallSpacing / 2 : Kirigami.Units.largeSpacing
        bottomPadding: NeoChatConfig.compactLayout ? Kirigami.Units.mediumSpacing / 2 : Kirigami.Units.largeSpacing
        leftPadding: NeoChatConfig.compactLayout ? 0 : Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing
        rightPadding: Kirigami.Units.largeSpacing + Kirigami.Units.smallSpacing

        author: root.author
        showAuthor: root.showAuthor
        isThreaded: root.isThreaded

        contentModel: root.contentModel

        showHighlight: root.showHighlight

        isPending: root.isPending

        onSelectedTextChanged: selectedText => {
            root.Message.selectedText = selectedText;
        }
        onHoveredLinkChanged: hoveredLink => {
            root.Message.hoveredLink = hoveredLink;
        }

        showBackground: root.cardBackground && !NeoChatConfig.compactLayout

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

    avatarComponent: KirigamiComponents.AvatarButton {
        id: avatar
        implicitWidth: Kirigami.Units.gridUnit + Kirigami.Units.largeSpacing * 2
        implicitHeight: width

        name: root.author.displayName
        source: root.author.avatarUrl
        color: root.author.color
        asynchronous: true
        QQC2.ToolTip.text: root.author.htmlSafeDisambiguatedName

        onClicked: RoomManager.resolveResource(root.author.uri)
    }

    sectionComponent: SectionDelegate {
        id: sectionDelegate
        labelText: root.section
        colorSet: root.compactMode ? Kirigami.Theme.View : Kirigami.Theme.Window
    }

    readMarkerComponent: AvatarFlow {
        model: root.readMarkers
    }

    compactBackgroundComponent: Rectangle {
        color: Kirigami.ColorUtils.tintWithAlpha(Kirigami.Theme.backgroundColor, Kirigami.Theme.highlightColor, 0.15)
        radius: Kirigami.Units.cornerRadius
    }

    // show hover actions
    onHoveredChanged: {
        if (hovered && !Kirigami.Settings.isMobile) {
            root.setHoverActionsToDelegate();
        }
    }

    function setHoverActionsToDelegate() {
        if (ListView.view.setHoverActionsToDelegate) {
            ListView.view.setHoverActionsToDelegate(root);
        }
    }

    QtObject {
        id: _private

        function showMessageMenu() {
            RoomManager.viewEventMenu(root.eventId, root.room, root.author, root.Message.selectedText, root.Message.hoveredLink);
        }
    }
}
