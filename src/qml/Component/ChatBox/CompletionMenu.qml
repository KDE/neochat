// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Popup {
    id: completionMenu
    width: parent.width

    visible: completions.count > 0

    RoomListModel {
        id: roomListModel
        connection: Controller.activeConnection
    }

    required property var chatDocumentHandler
    Component.onCompleted: {
        chatDocumentHandler.completionModel.roomListModel = roomListModel;
    }

    function incrementIndex() {
        completions.incrementCurrentIndex()
    }

    function decrementIndex() {
        completions.decrementCurrentIndex()
    }

    function complete() {
        completionMenu.chatDocumentHandler.complete(completions.currentIndex)
    }

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    implicitHeight: Math.min(completions.contentHeight, Kirigami.Units.gridUnit * 10)

    contentItem: ListView {
        id: completions

        anchors.fill: parent
        model: completionMenu.chatDocumentHandler.completionModel
        currentIndex: 0
        keyNavigationWraps: true
        highlightMoveDuration: 100
        delegate: Kirigami.BasicListItem {
            text: model.text
            subtitle: model.subtitle ?? ""
            labelItem.textFormat: Text.PlainText
            subtitleItem.textFormat: Text.PlainText
            leading: RowLayout {
                Kirigami.Avatar {
                    visible: model.icon !== "invalid"
                    Layout.preferredWidth: height
                    Layout.fillHeight: true
                    source: model.icon === "invalid" ? "" : ("image://mxc/" + model.icon)
                    name: model.text
                }
            }
            onClicked: completionMenu.chatDocumentHandler.complete(model.index)
        }
    }
    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }
}
