// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.de>
// SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import Qt.labs.qmlmodels

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

QQC2.Popup {
    id: root

    required property var chatDocumentHandler

    visible: completions.count > 0

    onVisibleChanged: if (visible) {
        root.open();
    }

    function incrementIndex() {
        completions.incrementCurrentIndex();
    }

    function decrementIndex() {
        completions.decrementCurrentIndex();
    }

    function complete() {
        root.chatDocumentHandler.insertCompletion(completions.currentItem.replacedText, completions.currentItem.hRef)
    }

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    implicitHeight: Math.min(completions.contentHeight, Kirigami.Units.gridUnit * 10)

    contentItem: ColumnLayout {
        spacing: 0
        Kirigami.Separator {
            Layout.fillWidth: true
        }
        QQC2.ScrollView {
            Layout.fillWidth: true
            Layout.preferredHeight: contentHeight
            Layout.maximumHeight: Kirigami.Units.gridUnit * 10

            background: Rectangle {
                color: Kirigami.Theme.backgroundColor
            }

            ListView {
                id: completions

                model: CompletionModel {
                    textItem: root.chatDocumentHandler.textItem
                    roomListModel: RoomManager.roomListModel
                    userListModel: RoomManager.userListModel
                }
                currentIndex: 0
                keyNavigationWraps: true
                highlightMoveDuration: 100
                onCountChanged: currentIndex = 0
                delegate: Delegates.RoundedItemDelegate {
                    id: completionDelegate

                    required property int index
                    required property string displayName
                    required property string subtitle
                    required property string iconName
                    required property string replacedText
                    required property url hRef

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
                            labelItem.clip: true // Intentional to limit insane Unicode in display names
                            subtitle: completionDelegate.subtitle ?? ""
                            subtitleItem.textFormat: Text.PlainText
                        }
                    }
                    onClicked: root.chatDocumentHandler.insertCompletion(replacedText, hRef)
                }
            }
        }
    }

    background: Rectangle {
        color: Kirigami.Theme.backgroundColor
    }
}
