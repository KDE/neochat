import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import Qt.labs.qmlmodels 1.0
import QtQuick.Controls.Material 2.12

import org.kde.kirigami 2.4 as Kirigami

import SortFilterProxyModel 0.2

import Spectral.Component 2.0
import Spectral.Component.Timeline 2.0
import Spectral.Dialog 2.0
import Spectral.Effect 2.0
import Spectral 0.1

Kirigami.ScrollablePage {
    property var currentRoom

    id: page

    title: "Messages"

    MessageEventModel {
        id: messageEventModel

        room: currentRoom
    }
    
    ImageClipboard {
        id: imageClipboard
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

                text: "Choose local file"

                onClicked: {
                    attachDialog.close()

                    var fileDialog = openFileDialog.createObject(ApplicationWindow.overlay)

                    fileDialog.chosen.connect(function(path) {
                        if (!path) return

                        roomPanelInput.attach(path)
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
                text: "Clipboard image"
                onClicked: {
                    var localPath = StandardPaths.writableLocation(StandardPaths.CacheLocation) + "/screenshots/" + (new Date()).getTime() + ".png"
                    if (!imageClipboard.saveImage(localPath)) return
                    roomPanelInput.attach(localPath)
                    attachDialog.close()
                }
            }
        }
    }

    Component {
        id: openFileDialog

        OpenFileDialog {}
    }


    SortFilterProxyModel {
        id: sortedMessageEventModel

        sourceModel: messageEventModel

        filters: [
            ExpressionFilter {
                expression: marks !== 0x10 && eventType !== "other"
            }
        ]
    }

    ListView {
        readonly property int largestVisibleIndex: count > 0 ? indexAt(contentX + (width / 2), contentY + height - 1) : -1
        readonly property bool noNeedMoreContent: !currentRoom || currentRoom.eventsHistoryJob || currentRoom.allHistoryLoaded

        readonly property bool isLoaded: page.width * page.height > 10

        id: messageListView

        spacing: Kirigami.Units.smallSpacing

        displayMarginBeginning: 100
        displayMarginEnd: 100
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

                        innerObject: TextDelegate {
                            Layout.fillWidth: true
                        }
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

                        innerObject: ImageDelegate {
                            Layout.preferredWidth: Math.min(320, info.w)
                            Layout.preferredHeight: Math.min(320, info.h)
                        }
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

        Component.onCompleted: {
            if (currentRoom) {
                if (currentRoom.timelineSize < 20)
                    currentRoom.getPreviousContent(50)
            }

            positionViewAtBeginning()
        }
    }

    footer: RoomPanelInput {
        id: roomPanelInput
        Layout.fillWidth: true
    }

    background: Item {}
}
