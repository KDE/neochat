// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import Qt.labs.platform

import org.kde.neochat

/**
 * @brief A timeline delegate for a poll message.
 *
 * @inherit MessageDelegate
 */
MessageDelegate {
    id: root

    /**
     * @brief The matrix message content.
     */
    required property var content

    /**
     * @brief The poll handler for this poll.
     *
     * This contains the required information like what the question, answers and
     * current number of votes for each is.
     */
    property var pollHandler: currentRoom.poll(root.eventId)

    bubbleContent: ColumnLayout {
        Label {
            id: questionLabel
            text: root.content["org.matrix.msc3381.poll.start"]["question"]["body"]
        }
        Repeater {
            model: root.content["org.matrix.msc3381.poll.start"]["answers"]
            delegate: RowLayout {
                CheckBox {
                    checked: root.pollHandler.answers[currentRoom.localUser.id] ? root.pollHandler.answers[currentRoom.localUser.id].includes(modelData["id"]) : false
                    onClicked: root.pollHandler.sendPollAnswer(root.eventId, modelData["id"])
                    enabled: !root.pollHandler.hasEnded
                }
                Label {
                    text: modelData["org.matrix.msc1767.text"]
                }
                Item {
                    Layout.fillWidth: true
                }
                Label {
                    visible: root.content["org.matrix.msc3381.poll.start"]["kind"] == "org.matrix.msc3381.poll.disclosed" || pollHandler.hasEnded
                    Layout.preferredWidth: contentWidth
                    text: root.pollHandler.counts[modelData["id"]] ?? "0"
                }
            }
        }
        Label {
            visible: root.content["org.matrix.msc3381.poll.start"]["kind"] == "org.matrix.msc3381.poll.disclosed" || root.pollHandler.hasEnded
            text: i18np("Based on votes by %1 user", "Based on votes by %1 users", root.pollHandler.answerCount) + (root.pollHandler.hasEnded ? (" " + i18nc("as in 'this vote has ended'", "(Ended)")) : "")
            font.pointSize: questionLabel.font.pointSize * 0.8
        }
    }
}
