// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.delegates 1.0 as Delegates
import org.kde.kirigamiaddons.labs.components 1.0 as Components

import org.kde.neochat 1.0

Delegates.RoundedItemDelegate {
    id: root

    required property string roomId
    required property string displayName
    required property url avatarUrl
    required property string alias
    required property string topic
    required property int memberCount
    required property bool isJoined
    property bool justJoined: false

    /**
     * @brief Signal emitted when a room delegate is selected.
     *
     * The signal contains all the delegate's model info so that it can be acted
     * upon as required, e.g. joining or entering the room or adding the room as
     * the child of a space.
     */
    signal roomSelected(string roomId,
                        string displayName,
                        url avatarUrl,
                        string alias,
                        string topic,
                        int memberCount,
                        bool isJoined)

    onClicked: {
        if (!isJoined) {
            justJoined = true;
        }
        root.roomSelected(root.roomId,
                          root.displayName,
                          root.avatarUrl,
                          root.alias,
                          root.topic,
                          root.memberCount,
                          root.isJoined)
    }

    contentItem: RowLayout {
        Components.Avatar {
            Layout.preferredWidth: Kirigami.Units.gridUnit * 2
            Layout.preferredHeight: Kirigami.Units.gridUnit * 2

            source: root.avatarUrl
            name: root.displayName
        }
        ColumnLayout {
            Layout.fillWidth: true
            RowLayout {
                Layout.fillWidth: true
                Kirigami.Heading {
                    Layout.fillWidth: true
                    level: 4
                    text: root.displayName
                    font.bold: true
                    textFormat: Text.PlainText
                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                }
                QQC2.Label {
                    visible: root.isJoined || root.justJoined
                    text: i18n("Joined")
                    color: Kirigami.Theme.linkColor
                }
            }
            QQC2.Label {
                Layout.fillWidth: true
                visible: text
                text: root.topic ? root.topic.replace(/(\r\n\t|\n|\r\t)/gm," ") : ""
                textFormat: Text.PlainText
                elide: Text.ElideRight
                wrapMode: Text.NoWrap
            }
            RowLayout {
                Layout.fillWidth: true
                Kirigami.Icon {
                    source: "user"
                    color: Kirigami.Theme.disabledTextColor
                    implicitHeight: Kirigami.Units.iconSizes.small
                    implicitWidth: Kirigami.Units.iconSizes.small
                }
                QQC2.Label {
                    text: root.memberCount + " " + (root.alias ?? root.roomId)
                    color: Kirigami.Theme.disabledTextColor
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }
            }
        }
    }
}
