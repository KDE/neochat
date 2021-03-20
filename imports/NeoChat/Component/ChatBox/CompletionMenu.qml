// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import org.kde.kirigami 2.14 as Kirigami

import org.kde.neochat 1.0

Popup {
    id: control

    // Expose internal ListView properties.
    property alias model: completionListView.model
    property alias currentIndex: completionListView.currentIndex
    property alias currentItem: completionListView.currentItem
    property alias count: completionListView.count
    property alias delegate: completionListView.delegate

    // Autocomplee text
    property string currentDisplayText: currentItem && currentItem.displayName ? currentItem.displayName : ""
    property string currentUserId: currentItem && currentItem.id ? currentItem.id : ""

    property bool isCompletingEmoji: false
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

    implicitHeight: Math.min(completionListView.contentHeight, Kirigami.Units.gridUnit * 5)

    contentItem: ScrollView {
        ListView {
            id: completionListView
            implicitWidth: contentWidth
            delegate: isCompletingEmoji ? emojiDelegate : usernameDelegate

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
            leading: Kirigami.Avatar {
                implicitHeight: Kirigami.Units.gridUnit
                implicitWidth: implicitHeight
                source: modelData.avatarMediaId ? ("image://mxc/" + modelData.avatarMediaId) : ""
                color: modelData.color ? Qt.darker(modelData.color, 1.1) : null
            }
            text: modelData.displayName
            onClicked: completeTriggered();
            Component.onCompleted: {
                completionMenu.currentUserId = Qt.binding(() => {
                    return modelData.id ?? "";
                });
            }
        }
    }

    Component {
        id: emojiDelegate
        Kirigami.BasicListItem {
            id: emojiItem
            width: ListView.view.width ?? implicitWidth
            property string displayName: modelData.unicode
            text: modelData.unicode + " " + modelData.shortname

            leading: Label {
                id: unicodeLabel
                Layout.preferredHeight: Kirigami.Units.gridUnit
                Layout.preferredWidth: textMetrics.tightBoundingRect.width
                font.pointSize: Kirigami.Units.gridUnit * 0.75
                text: modelData.unicode
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
            TextMetrics {
                id: textMetrics
                text: modelData.unicode
                font: unicodeLabel.font
            }
            onClicked: completeTriggered();
        }
    }
}
