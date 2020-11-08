/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.13 as Kirigami

import NeoChat.Dialog 2.0

Kirigami.OverlaySheet {
    id: root

    required property var author;
    required property string message;
    required property string eventId;

    signal viewSource()
    signal reply(var author, string message)
    signal remove()

    parent: applicationWindow().overlay

    leftPadding: 0
    rightPadding: 0

    ColumnLayout {
        spacing: 0
        QQC2.Control {
            leftPadding: Kirigami.Units.largeSpacing
            rightPadding: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            implicitHeight: headerLayout.implicitHeight
            contentItem: RowLayout {
                id: headerLayout
                Layout.fillWidth: true
                spacing: Kirigami.Units.largeSpacing
                Kirigami.Avatar {
                    source: author.avatarMediaId ? "image://mxc/" + author.avatarMediaId : ""
                    Layout.preferredWidth: Kirigami.Units.iconSizes.large
                    Layout.preferredHeight: Kirigami.Units.iconSizes.large
                    Layout.alignment: Qt.AlignTop
                }
                ColumnLayout {
                    Layout.fillWidth: true
                    Kirigami.Heading {
                        level: 3
                        Layout.fillWidth: true
                        text: author.displayName
                    }
                    QQC2.Label {
                        text: message
                        Layout.fillWidth: true
                    }
                }
            }
        }
        Row {
            spacing: 0
            Repeater {
                model: ["👍", "👎️", "😄", "🎉", "🚀", "👀"]
                delegate: QQC2.ItemDelegate {
                    width: 32
                    height: 32

                    contentItem: QQC2.Label {
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter

                        font.pixelSize: 16
                        font.family: "emoji"
                        text: modelData

                    }

                    onClicked: {
                        currentRoom.toggleReaction(eventId, modelData)
                        root.close();
                    }
                }
            }
        }
        Kirigami.BasicListItem {
            action: Kirigami.Action {
                text: i18n("Reply")
                icon.name: "reply"
                onTriggered: reply(author, message)
            }
        }
        Kirigami.BasicListItem {
            action: Kirigami.Action {
                text: i18n("Remove")
                icon.name: "edit-delete-remove"
                icon.color: "red"
                onTriggered: remove()
            }
        }
        Kirigami.BasicListItem {
            action: Kirigami.Action {
                text: i18n("Copy")
                icon.name: "copy"
                onTriggered: Clibpoard.setText(message)
            }
        }
        Kirigami.BasicListItem {
            action: Kirigami.Action {
                text: i18n("View Source")
                icon.name: "code-context"
                onTriggered: viewSource()
            }
        }
    }
}
