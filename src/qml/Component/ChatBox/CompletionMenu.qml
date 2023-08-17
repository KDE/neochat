// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15 as QQC2
import Qt.labs.qmlmodels 1.0

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.delegates 1.0 as Delegates
import org.kde.kirigamiaddons.labs.components 1.0 as KirigamiComponents

import org.kde.neochat 1.0

QQC2.Popup {
    id: completionMenu
    width: parent.width

    visible: completions.count > 0

    RoomListModel {
        id: roomListModel
        connection: Controller.activeConnection
    }

    property var chatDocumentHandler
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
        delegate: Delegates.RoundedItemDelegate {
            id: completionDelegate

            required property int index
            required property string displayName
            required property string subtitle
            required property string iconName

            text: displayName

            contentItem: RowLayout {
                KirigamiComponents.Avatar {
                    visible: completionDelegate.iconName !== "invalid"
                    Layout.preferredWidth: Kirigami.Units.iconSizes.medium
                    Layout.preferredHeight: Kirigami.Units.iconSizes.medium
                    source: completionDelegate.iconName === "invalid" ? "" : completionDelegate.iconName
                    name: completionDelegate.text
                }
                Delegates.SubtitleContentItem {
                    itemDelegate: completionDelegate
                    labelItem.textFormat: Text.PlainText
                    subtitle: completionDelegate.subtitle ?? ""
                    subtitleItem.textFormat: Text.PlainText
                }
            }
            onClicked: completionMenu.chatDocumentHandler.complete(completionDelegate.index)
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }
}
