// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.neochat

/**
 * @brief A component to show a poll from a message.
 */
ColumnLayout {
    id: root

    /**
     * @brief The NeoChatRoom the delegate is being displayed in.
     */
    required property NeoChatRoom room

    /**
     * @brief The matrix ID of the message event.
     */
    required property string eventId

    /**
     * @brief The poll handler for this poll.
     *
     * This contains the required information like what the question, answers and
     * current number of votes for each is.
     */
    required property var pollHandler

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth

    spacing: 0

    Label {
        id: questionLabel
        text: root.pollHandler.question
        wrapMode: Text.Wrap
        Layout.fillWidth: true
        visible: text.length > 0
    }

    Repeater {
        model: root.pollHandler.options
        delegate: FormCard.FormCheckDelegate {
            Layout.fillWidth: true
            Layout.leftMargin: -Kirigami.Units.largeSpacing - Kirigami.Units.smallSpacing
            Layout.rightMargin: -Kirigami.Units.largeSpacing - Kirigami.Units.smallSpacing

            checked: root.pollHandler.answers[root.room.localMember.id] ? root.pollHandler.answers[root.room.localMember.id].includes(modelData["id"]) : false
            onClicked: root.pollHandler.sendPollAnswer(root.eventId, modelData["id"])
            enabled: !root.pollHandler.hasEnded
            text: modelData["org.matrix.msc1767.text"]

            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing

            trailing: Label {
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
