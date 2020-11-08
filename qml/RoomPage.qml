import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import Qt.labs.qmlmodels 1.0
import QtQuick.Controls.Material 2.12

import org.kde.kirigami 2.13 as Kirigami
import org.kde.kitemmodels 1.0
import org.kde.neochat 1.0

import Spectral.Component 2.0
import Spectral.Component.Timeline 2.0
import Spectral.Dialog 2.0
import Spectral.Effect 2.0
import Spectral.Menu.Timeline 2.0
import Spectral 0.1

Kirigami.ScrollablePage {
    id: page

    property var currentRoom

    title: i18n("Messages")

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
                    var localPath = StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/screenshots/" + (new Date()).getTime() + ".png"
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

    ListView {
        readonly property int largestVisibleIndex: count > 0 ? indexAt(contentX + (width / 2), contentY + height - 1) : -1
        readonly property bool noNeedMoreContent: !currentRoom || currentRoom.eventsHistoryJob || currentRoom.allHistoryLoaded

        readonly property bool isLoaded: page.width * page.height > 10

        id: messageListView

        spacing: Kirigami.Units.smallSpacing
        clip: true

        displayMarginBeginning: Kirigami.Units.gridUnit
        displayMarginEnd: typingNotification.visible ? typingNotification.height : Kirigami.Units.gridUnit
        verticalLayoutDirection: ListView.BottomToTop
        highlightMoveDuration: 500

        model: !isLoaded ? undefined : sortedMessageEventModel

        onContentYChanged: {
            if(!noNeedMoreContent && contentY  - 5000 < originY)
                currentRoom.getPreviousContent(20);
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

                    innerObject: StateDelegate {
                        Layout.maximumWidth: parent.width
                        Layout.alignment: Qt.AlignHCenter
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
                        innerObject: [
                            TextDelegate {
                                Layout.fillWidth: true
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
                roleValue: "notice"
                delegate: TimelineContainer {
                    width: messageListView.width

                    innerObject: MessageDelegate {
                        Layout.fillWidth: true

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

                        innerObject: [
                            ImageDelegate {
                                Layout.maximumWidth: parent.width
                                Layout.preferredWidth: Math.min(320, info.w)
                                Layout.preferredHeight: Math.min(320, info.h)
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

        QQC2.Button {
            anchors.right: parent.right
            anchors.top: parent.top
            anchors.topMargin: 16

            padding: 8

            id: goReadMarkerFab

            visible: currentRoom && currentRoom.hasUnreadMessages || !messageListView.atYEnd
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
        }

        QQC2.Control {
            id: typingNotification
            anchors.left: parent.left
            anchors.bottom: parent.bottom

            visible: currentRoom && currentRoom.usersTyping.length > 0
            padding: 4

            contentItem: RowLayout {
                spacing: Kirigami.Units.largeSpacing

                QQC2.BusyIndicator {
                    Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.smallMedium
                }
                QQC2.Label {
                    text: i18ncp("Message displayed when some users are typing", "%2 is typing", "%2 are typing", currentRoom.usersTyping.length, currentRoom.usersTyping.join(", "))
                }
            }
        }


        Component.onCompleted: {
            if (currentRoom) {
                if (currentRoom.timelineSize < 20)
                    currentRoom.getPreviousContent(50)
            }

            positionViewAtBeginning()
        }
    }

    footer: ChatTextInput {
        id: chatTextInput
        Layout.fillWidth: true
    }

    background: Item {}

    function openMessageContext(author, message, eventId, toolTip, model) {
        const contextMenu = messageDelegateContextMenu.createObject(root, {
            'author': author,
            'message': message,
            'eventId': eventId,
        });
        contextMenu.viewSource.connect(function() {
            messageSourceDialog.createObject(root, {
                'sourceText': toolTip,
            }).open();
            contextMenu.close();
        });
        contextMenu.reply.connect(function(replyUser, replyContent) {
            chatTextInput.replyUser = replyUser;
            chatTextInput.replyEventID = eventId;
            chatTextInput.replyContent = replyContent;
            chatTextInput.isReply = true;
            chatTextInput.focus();
            contextMenu.close();
        })
        contextMenu.remove.connect(function() {
            currentRoom.redactEvent(eventId);
            contextMenu.close();
        })
        contextMenu.open()
    }

    Component {
        id: messageDelegateContextMenu

        MessageDelegateContextMenu {}
    }

    Component {
        id: messageSourceDialog

        MessageSourceDialog {}
    }
}
