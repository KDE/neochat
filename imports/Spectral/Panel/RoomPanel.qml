import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.4 as Kirigami

import SortFilterProxyModel 0.2

import Spectral.Component 2.0
import Spectral.Component.Timeline 2.0
import Spectral 0.1

Kirigami.ScrollablePage {
    property var currentRoom

    id: page

    title: "Messages"

    actions {
        main: Kirigami.Action {
            iconName: "document-edit"
        }
        contextualActions: []
    }

    MessageEventModel {
        id: messageEventModel

        room: currentRoom
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
                    width: page.width

                    StateDelegate {}
                }
            }

            DelegateChoice {
                roleValue: "emote"
                delegate: TimelineContainer {
                    width: page.width

                    innerObject: StateDelegate {}
                }
            }

            DelegateChoice {
                roleValue: "message"
                delegate: TimelineContainer {
                    width: page.width

                    innerObject: MessageDelegate {
                        width: parent.width

                        innerObject: TextDelegate {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "notice"
                delegate: TimelineContainer {
                    width: page.width

                    innerObject: MessageDelegate {
                        width: parent.width

                        innerObject: TextDelegate {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "image"
                delegate: TimelineContainer {
                    width: page.width

                    innerObject: MessageDelegate {
                        width: parent.width

                        innerObject: ImageDelegate {
                            Layout.fillWidth: true
                            Layout.preferredHeight: info.h
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "audio"
                delegate: TimelineContainer {
                    width: page.width

                    innerObject: MessageDelegate {
                        width: parent.width

                        innerObject: AudioDelegate {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "video"
                delegate: TimelineContainer {
                    width: page.width

                    innerObject: MessageDelegate {
                        width: parent.width

                        innerObject: AudioDelegate {
                            Layout.fillWidth: true
                        }
                    }
                }
            }

            DelegateChoice {
                roleValue: "file"
                delegate: TimelineContainer {
                    width: page.width

                    innerObject: MessageDelegate {
                        width: parent.width

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

    footer: RowLayout {
        Controls.ToolButton {
            contentItem: Kirigami.Icon {
                source: "mail-attachment"
            }
        }

        Controls.TextField {
            Layout.fillWidth: true
        }
    }

    background: Item {}
}
