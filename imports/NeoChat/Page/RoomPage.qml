/**
 * SPDX-FileCopyrightText: 2018-2020 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import Qt.labs.qmlmodels 1.0
import Qt.labs.platform 1.0 as Platform
import QtQuick.Controls.Material 2.12

import org.kde.kirigami 2.13 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.neochat 1.0

import NeoChat.Component 1.0
import NeoChat.Component.Timeline 1.0
import NeoChat.Dialog 1.0
import NeoChat.Effect 1.0
import NeoChat.Menu.Timeline 1.0

Kirigami.ScrollablePage {
    id: page

    property var currentRoom

    title: currentRoom.name

    ListView {
        MessageEventModel {
            id: messageEventModel

            room: currentRoom
        }

        QQC2.Popup {
            anchors.centerIn: parent

            id: attachDialog

            padding: 16

            contentItem: RowLayout {
                QQC2.ToolButton {
                    Layout.preferredWidth: 160
                    Layout.fillHeight: true

                    icon.name: 'mail-attachment'

                    text: i18n("Choose local file")

                    onClicked: {
                        attachDialog.close()

                        var fileDialog = openFileDialog.createObject(ApplicationWindow.overlay)

                        fileDialog.chosen.connect(function(path) {
                            if (!path) return

                            chatTextInput.attach(path)
                        })

                        fileDialog.open()
                    }
                }

                Kirigami.Separator {}

                QQC2.ToolButton {
                    Layout.preferredWidth: 160
                    Layout.fillHeight: true

                    padding: 16

                    icon.name: 'insert-image'
                    text: i18n("Clipboard image")
                    onClicked: {
                        var localPath = Platform.StandardPaths.writableLocation(Platform.StandardPaths.CacheLocation) + "/screenshots/" + (new Date()).getTime() + ".png"
                        if (!Clipboard.saveImage(localPath)) return
                        chatTextInput.attach(localPath)
                        attachDialog.close()
                    }
                }
            }
        }

        Component {
            id: openFileDialog

            OpenFileDialog {}
        }


        KSortFilterProxyModel {
            id: sortedMessageEventModel

            sourceModel: messageEventModel

            filterRowCallback: function(row, parent) {
                return messageEventModel.data(messageEventModel.index(row, 0), MessageEventModel.MessageRole) !== 0x10 && messageEventModel.data(messageEventModel.index(row, 0), MessageEventModel.EventTypeRole) !== "other"
            }
        }
        readonly property int largestVisibleIndex: count > 0 ? indexAt(contentX + (width / 2), contentY + height - 1) : -1
        readonly property bool noNeedMoreContent: !currentRoom || currentRoom.eventsHistoryJob || currentRoom.allHistoryLoaded

        readonly property bool isLoaded: page.width * page.height > 10

        id: messageListView

        spacing: Kirigami.Units.smallSpacing
        clip: true

        verticalLayoutDirection: ListView.BottomToTop
        highlightMoveDuration: 500

        model: !isLoaded ? undefined : sortedMessageEventModel

        onContentYChanged: {
            if(!noNeedMoreContent && contentY  - 5000 < originY)
                currentRoom.getPreviousContent(20);
            const index = eventToIndex(currentRoom.readMarkerEventId)
            if(index === -1)
                return
            if(firstVisibleIndex() === -1 || lastVisibleIndex() === -1)
                return
            if(index < firstVisibleIndex() && index > lastVisibleIndex()) {
                currentRoom.readMarkerEventId = sortedMessageEventModel.data(sortedMessageEventModel.index(lastVisibleIndex(), 0), MessageEventModel.EventIdRole)
            }
        }

        //        populate: Transition {
        //            NumberAnimation {
        //                property: "opacity"; from: 0; to: 1
        //                duration: 200
        //            }
        //        }

        //        add: Transition {
        //            NumberAnimation {
        //                property: "opacity"; from: 0; to: 1
        //                duration: 200
        //            }
        //        }

        //        move: Transition {
        //            NumberAnimation {
        //                property: "y"; duration: 200
        //            }
        //            NumberAnimation {
        //                property: "opacity"; to: 1
        //            }
        //        }

        //        displaced: Transition {
        //            NumberAnimation {
        //                property: "y"; duration: 200
        //                easing.type: Easing.OutQuad
        //            }
        //            NumberAnimation {
        //                property: "opacity"; to: 1
        //            }
        //        }

        delegate: DelegateChooser {
            id: timelineDelegateChooser
            role: "eventType"

            DelegateChoice {
                roleValue: "state"
                delegate: TimelineContainer {
                    width: messageListView.width

                    innerObject: StateDelegate {
                        Layout.maximumWidth: parent.width
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }

            DelegateChoice {
                roleValue: "emote"
                delegate: TimelineContainer {
                    width: messageListView.width
                    innerObject: MessageDelegate {
                        Layout.fillWidth: true
                        Layout.maximumWidth: messageListView.width
                        isEmote: true
                        mouseArea: MouseArea {
                            acceptedButtons: Qt.RightButton
                            anchors.fill: parent
                            onClicked: openMessageContext(author, display, eventId, toolTip);
                        }
                        onReplyClicked: goToEvent(eventID)
                        onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);
                        innerObject: [
                            TextDelegate {
                                isEmote: true
                                Layout.fillWidth: true
                                Layout.rightMargin: Kirigami.Units.largeSpacing
                            },
                            ReactionDelegate {
                                Layout.fillWidth: true
                                Layout.topMargin: 0
                                Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                            }
                        ]
                    }
                }
            }

            DelegateChoice {
                roleValue: "message"
                delegate: TimelineContainer {
                    width: messageListView.width
                    innerObject: MessageDelegate {
                        Layout.fillWidth: true
                        Layout.maximumWidth: messageListView.width
                        mouseArea: MouseArea {
                            acceptedButtons: Qt.RightButton
                            anchors.fill: parent
                            onClicked: openMessageContext(author, display, eventId, toolTip);
                        }
                        onReplyClicked: goToEvent(eventID)
                        onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);
                        innerObject: [
                            TextDelegate {
                                Layout.fillWidth: true
                                Layout.rightMargin: Kirigami.Units.largeSpacing
                            },
                            ReactionDelegate {
                                Layout.fillWidth: true
                                Layout.topMargin: 0
                                Layout.bottomMargin: Kirigami.Units.largeSpacing * 2
                            }
                        ]
                    }
                }
            }

            DelegateChoice {
                roleValue: "notice"
                delegate: TimelineContainer {
                    width: messageListView.width

                    innerObject: MessageDelegate {
                        Layout.fillWidth: true
                        onReplyClicked: goToEvent(eventID)
                        onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);

                        innerObject: TextDelegate {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "image"
                delegate: TimelineContainer {
                    width: messageListView.width

                    innerObject: MessageDelegate {
                        Layout.fillWidth: true
                        onReplyClicked: goToEvent(eventID)
                        onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);

                        innerObject: [
                            ImageDelegate {
                                Layout.maximumWidth: parent.width
                                Layout.minimumWidth: 320
                            },
                            ReactionDelegate {
                                Layout.fillWidth: true
                                Layout.topMargin: 0
                                Layout.bottomMargin: 8
                            }
                        ]
                    }
                }
            }

            DelegateChoice {
                roleValue: "audio"
                delegate: TimelineContainer {
                    width: messageListView.width

                    innerObject: MessageDelegate {
                        Layout.fillWidth: true
                        onReplyClicked: goToEvent(eventID)
                        onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);

                        innerObject: AudioDelegate {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "video"
                delegate: TimelineContainer {
                    width: messageListView.width

                    innerObject: MessageDelegate {
                        Layout.fillWidth: true
                        onReplyClicked: goToEvent(eventID)
                        onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);

                        innerObject: AudioDelegate {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "file"
                delegate: TimelineContainer {
                    width: messageListView.width

                    innerObject: MessageDelegate {
                        Layout.fillWidth: true
                        onReplyClicked: goToEvent(eventID)
                        onReplyToMessageClicked: replyToMessage(replyUser, replyContent, eventId);

                        innerObject: FileDelegate {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "other"
                delegate: Item {}
            }
        }

        QQC2.RoundButton {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: Kirigami.Units.largeSpacing
            anchors.rightMargin: Kirigami.Units.largeSpacing

            padding: 8

            id: goReadMarkerFab

            visible: currentRoom && currentRoom.hasUnreadMessages && currentRoom.readMarkerLoaded || !messageListView.atYEnd
            action: Kirigami.Action {
                onTriggered: {
                    if (currentRoom && currentRoom.hasUnreadMessages) {
                        goToEvent(currentRoom.readMarkerEventId)
                    } else {
                        currentRoom.markAllMessagesAsRead()
                        messageListView.positionViewAtBeginning()
                    }
                }
                icon.name: currentRoom && currentRoom.hasUnreadMessages ? "go-up" : "go-down"
            }

            QQC2.ToolTip {
                text: currentRoom && currentRoom.hasUnreadMessages ? i18n("Jump to first unread message") : i18n("Jump to latest message")
            }
        }

        header: RowLayout {
            id: typingNotification

            visible: currentRoom && currentRoom.usersTyping.length > 0
            spacing: Kirigami.Units.largeSpacing

            QQC2.BusyIndicator {
                Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
            }
            QQC2.Label {
                text: visible ? i18ncp("Message displayed when some users are typing", "%2 is typing", "%2 are typing", currentRoom.usersTyping.length, currentRoom.usersTyping.map(user => user.displayName).join(", ")) : ""
            }
        }


        Component.onCompleted: {
            if (currentRoom) {
                if (currentRoom.timelineSize < 20)
                    currentRoom.getPreviousContent(50)
            }

            positionViewAtBeginning()
        }

        DropArea {
            id: dropAreaFile
            anchors.fill: parent
            onDropped: chatTextInput.attach(drop.urls[0])
        }

        QQC2.Pane {
            visible: dropAreaFile.containsDrag
            anchors {
                fill: parent
                margins: Kirigami.Units.gridUnit
            }

            Kirigami.PlaceholderMessage {
                anchors.centerIn: parent
                width: parent.width - (Kirigami.Units.largeSpacing * 4)
                text: i18n("Drag items here to share them")
            }
        }

        Component {
            id: messageDelegateContextMenu

            MessageDelegateContextMenu {}
        }

        Component {
            id: messageSourceSheet

            MessageSourceSheet {}
        }
    }

    footer: ChatTextInput {
        id: chatTextInput
        Layout.fillWidth: true
    }

    Kirigami.Theme.colorSet: Kirigami.Theme.View

    function goToEvent(eventID) {
        messageListView.positionViewAtIndex(eventToIndex(eventID), ListView.Contain)
    }

    function eventToIndex(eventID) {
        const index = messageEventModel.eventIDToIndex(eventID)
        if (index === -1)
            return -1
        return sortedMessageEventModel.mapFromSource(messageEventModel.index(index, 0)).row
    }

    function firstVisibleIndex() {
        let center = messageListView.x + messageListView.width / 2;
        return messageListView.indexAt(center, messageListView.y + messageListView.contentY);
    }

    function lastVisibleIndex() {
        let center = messageListView.x + messageListView.width / 2;
        return messageListView.indexAt(center, messageListView.y + messageListView.contentY + messageListView.height);
    }

    function openMessageContext(author, message, eventId, toolTip, model) {
        const contextMenu = messageDelegateContextMenu.createObject(page, {
            'author': author,
            'message': message,
            'eventId': eventId,
        });
        contextMenu.viewSource.connect(function() {
            messageSourceSheet.createObject(page, {
                'sourceText': toolTip,
            }).open();
            contextMenu.close();
        });
        contextMenu.reply.connect(function(replyUser, replyContent) {
            replyToMessage(replyUser, replyContent, eventId);
            contextMenu.close();
        })
        contextMenu.remove.connect(function() {
            currentRoom.redactEvent(eventId);
            contextMenu.close();
        })
        contextMenu.open()
    }

    function replyToMessage(replyUser, replyContent, eventId) {
        chatTextInput.replyUser = replyUser;
        chatTextInput.replyEventID = eventId;
        chatTextInput.replyContent = replyContent;
        chatTextInput.isReply = true;
        chatTextInput.focus();
    }
}
