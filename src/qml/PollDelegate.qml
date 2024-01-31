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
     * @brief The poll handler for this poll.
     *
     * This contains the required information like what the question, answers and
     * current number of votes for each is.
     */
    required property var pollHandler

    bubbleContent: ColumnLayout {
        Label {
            id: questionLabel
            text: root.pollHandler.question
            wrapMode: Text.Wrap
            Layout.fillWidth: true
        }
        Repeater {
            model: root.pollHandler.options
            delegate: RowLayout {
                Layout.fillWidth: true
                CheckBox {
                    checked: root.pollHandler.answers[root.room.localUser.id] ? root.pollHandler.answers[root.room.localUser.id].includes(modelData["id"]) : false
                    onClicked: root.pollHandler.sendPollAnswer(root.eventId, modelData["id"])
                    enabled: !root.pollHandler.hasEnded
                }
                Label {
                    text: modelData["org.matrix.msc1767.text"]
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                }
                Label {
                    visible: root.pollHandler.kind == "org.matrix.msc3381.poll.disclosed" || pollHandler.hasEnded
                    Layout.preferredWidth: contentWidth
                    text: root.pollHandler.counts[modelData["id"]] ?? "0"
                }
            }
        }
        Label {
            visible: root.pollHandler.kind == "org.matrix.msc3381.poll.disclosed" || root.pollHandler.hasEnded
            text: i18np("Based on votes by %1 user", "Based on votes by %1 users", root.pollHandler.answerCount) + (root.pollHandler.hasEnded ? (" " + i18nc("as in 'this vote has ended'", "(Ended)")) : "")
            font.pointSize: questionLabel.font.pointSize * 0.8
        }
    }
}
