// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.kirigamiaddons.formcard as FormCard

import Quotient

import org.kde.neochat

/**
 * @brief A component to show a poll from a message.
 */
ColumnLayout {
    id: root

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

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth
    Layout.minimumWidth: Message.maxContentWidth

    spacing: 0

    RowLayout {
        Layout.fillWidth: true

        Kirigami.Icon {
            implicitWidth: Kirigami.Units.iconSizes.smallMedium
            implicitHeight: implicitWidth
            source: "amarok_playcount"
        }
        QQC2.Label {
            id: questionLabel
            Layout.fillWidth: true
            topPadding: Kirigami.Units.largeSpacing
            bottomPadding: Kirigami.Units.largeSpacing

            text: root.pollHandler.question
            wrapMode: Text.Wrap
            visible: text.length > 0
        }
    }
    Repeater {
        model: root.pollHandler.answerModel
        delegate: Delegates.RoundedItemDelegate {
            id: answerDelegate

            required property string id
            required property string answerText
            required property int count
            required property bool localChoice
            required property bool isWinner

            Layout.fillWidth: true
            Layout.leftMargin: -Kirigami.Units.largeSpacing - Kirigami.Units.smallSpacing
            Layout.rightMargin: -Kirigami.Units.largeSpacing - Kirigami.Units.smallSpacing

            highlighted: false

            onClicked: {
                if (root.pollHandler.hasEnded) {
                    return;
                }
                root.pollHandler.sendPollAnswer(root.eventId, answerDelegate.id);
            }
            text: answerDelegate.answerText

            topPadding: Kirigami.Units.smallSpacing
            bottomPadding: Kirigami.Units.smallSpacing

            contentItem: ColumnLayout {
                Layout.fillWidth: true

                RowLayout {
                    Layout.fillWidth: true

                    QQC2.CheckBox {
                        enabled: !root.pollHandler.hasEnded
                        checked: answerDelegate.localChoice

                        onClicked: answerDelegate.clicked()
                    }
                    QQC2.Label {
                        Layout.fillWidth: true

                        text: answerDelegate.answerText
                    }
                    Kirigami.Icon {
                        implicitWidth: Kirigami.Units.iconSizes.small
                        implicitHeight: implicitWidth
                        visible: answerDelegate.isWinner
                        source: "favorite-favorited"
                    }
                    QQC2.Label {
                        visible: root.pollHandler.kind == PollKind.Disclosed || pollHandler.hasEnded
                        horizontalAlignment: Text.AlignRight
                        text: i18np("%1 Vote", "%1 Votes", answerDelegate.count)
                    }
                }
                QQC2.ProgressBar {
                    id: voteProgress

                    Layout.fillWidth: true
                    to: root.pollHandler.totalCount
                    value: root.pollHandler.kind == PollKind.Disclosed || pollHandler.hasEnded ? answerDelegate.count : 0
                }
            }
        }
    }

    QQC2.Label {
        visible: root.pollHandler.kind == PollKind.Disclosed || root.pollHandler.hasEnded
        text: i18np("Based on votes by %1 user", "Based on votes by %1 users", root.pollHandler.totalCount) + (root.pollHandler.hasEnded ? (" " + i18nc("as in 'this vote has ended'", "(Ended)")) : "")
        font.pointSize: questionLabel.font.pointSize * 0.8
    }
}
