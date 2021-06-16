// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0

Popup {
    id: control

    // Expose internal ListView properties.
    property alias model: completionListView.model
    property alias listView: completionListView
    property alias currentIndex: completionListView.currentIndex
    property alias currentItem: completionListView.currentItem
    property alias count: completionListView.count
    property alias delegate: completionListView.delegate

    // Autocomplee text
    property string currentDisplayText: currentItem && (currentItem.displayName ?? "")

    property int completionType: ChatDocumentHandler.Emoji
    property int beginPosition: 0
    property int endPosition: 0

    signal completeTriggered()

    Kirigami.Theme.colorSet: Kirigami.Theme.View

    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    clip: true

    onVisibleChanged: if (!visible) {
        completionListView.currentIndex = 0;
    }

    implicitHeight: Math.min(completionListView.contentHeight, Kirigami.Units.gridUnit * 10)

    contentItem: ScrollView {
        ScrollBar.horizontal.policy: ScrollBar.AlwaysOff
        ListView {
            id: completionListView
            implicitWidth: contentWidth
            delegate: {
                if (completionType === ChatDocumentHandler.Emoji) {
                    emojiDelegate
                } else if (completionType === ChatDocumentHandler.Command) {
                    commandDelegate
                } else if (completionType === ChatDocumentHandler.User) {
                    usernameDelegate
                }
            }

            keyNavigationWraps: true

            //interactive: Window.window ? contentHeight + control.topPadding + control.bottomPadding > Window.window.height : false
            clip: true
            currentIndex: control.currentIndex || 0
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }

    Component {
        id: usernameDelegate
        Kirigami.BasicListItem {
            id: usernameItem
            width: ListView.view.width ?? implicitWidth
            property string displayName: modelData.displayName
            property string userId: modelData.id
            leading: Kirigami.Avatar {
                implicitHeight: Kirigami.Units.gridUnit
                implicitWidth: implicitHeight
                source: modelData.avatarMediaId ? ("image://mxc/" + modelData.avatarMediaId) : ""
                color: modelData.color ? Qt.darker(modelData.color, 1.1) : null
            }
            text: modelData.displayName
            onClicked: completeTriggered();
        }
    }

    Component {
        id: emojiDelegate
        Kirigami.BasicListItem {
            id: emojiItem
            width: ListView.view.width ?? implicitWidth
            property string displayName: modelData.isCustom ? modelData.shortname : modelData.unicode
            text: modelData.shortname
            height: Kirigami.Units.gridUnit * 2

            leading: Image {
                source: modelData.isCustom ? modelData.unicode : ""

                width: height
                sourceSize.width: width
                sourceSize.height: height

                Rectangle {
                    anchors.fill: parent
                    visible: parent.status === Image.Loading
                    radius: height/2
                    gradient: ShimmerGradient { }
                }

                Label {
                    id: unicodeLabel

                    visible: !modelData.isCustom

                    font.family: 'emoji'
                    font.pixelSize: height - 2

                    text: modelData.unicode
                    horizontalAlignment: Text.AlignHCenter
                    verticalAlignment: Text.AlignVCenter

                    anchors.fill: parent
                }
            }

            onClicked: completeTriggered();
        }
    }

    Component {
        id: commandDelegate
        Kirigami.BasicListItem {
            id: commandItem
            width: ListView.view.width ?? implicitWidth
            text: "<i>" + modelData.parameter.replace("<", "&lt;").replace(">", "&gt;") + "</i> " +  modelData.help
            property string displayName: modelData.command

            leading: Label {
                id: commandLabel
                Layout.preferredHeight: Kirigami.Units.gridUnit
                Layout.preferredWidth: textMetrics.tightBoundingRect.width
                text: modelData.command
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            TextMetrics {
                id: textMetrics
                text: modelData.command
                font: commandLabel.font
            }
            onClicked: completeTriggered();
        }
    }
}
