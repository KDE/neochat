// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import Qt.labs.platform 1.1

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

TimelineContainer {
    id: pollDelegate

    readonly property var data: model
    property var pollHandler: currentRoom.poll(model.eventId)

    innerObject: ColumnLayout {
        Label {
            id: questionLabel
            text: pollDelegate.data.content["org.matrix.msc3381.poll.start"]["question"]["body"]
        }
        Repeater {
            model: pollDelegate.data.content["org.matrix.msc3381.poll.start"]["answers"]
            delegate: RowLayout {
                width: pollDelegate.innerObject.width
                CheckBox {
                    checked: pollDelegate.pollHandler.answers[currentRoom.localUser.id] ? pollDelegate.pollHandler.answers[currentRoom.localUser.id].includes(modelData["id"]) : false
                    onClicked: pollDelegate.pollHandler.sendPollAnswer(pollDelegate.data.eventId, modelData["id"])
                    enabled: !pollDelegate.pollHandler.hasEnded
                }
                Label {
                    text: modelData["org.matrix.msc1767.text"]
                }
                Item {
                    Layout.fillWidth: true
                }
                Label {
                    visible: pollDelegate.data.content["org.matrix.msc3381.poll.start"]["kind"] == "org.matrix.msc3381.poll.disclosed" || pollHandler.hasEnded
                    Layout.preferredWidth: contentWidth
                    text: pollDelegate.pollHandler.counts[modelData["id"]] ?? "0"
                }
            }
        }
        Label {
            visible: pollDelegate.data.content["org.matrix.msc3381.poll.start"]["kind"] == "org.matrix.msc3381.poll.disclosed" || pollDelegate.pollHandler.hasEnded
            text: i18np("Based on votes by %1 user", "Based on votes by %1 users", pollDelegate.pollHandler.answerCount) + (pollDelegate.pollHandler.hasEnded ? (" " + i18nc("as in 'this vote has ended'", "(Ended)")) : "")
            font.pointSize: questionLabel.font.pointSize * 0.8
        }
    }
}
