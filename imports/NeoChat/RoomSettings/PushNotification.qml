// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {

    property var room

    title: i18nc('@title:window', 'Notifications')

    ColumnLayout {
        Kirigami.FormLayout {
            Layout.fillWidth: true

            QQC2.RadioButton {
                text: i18n("Follow global setting")
                Kirigami.FormData.label: i18n("Room notifications setting:")
                checked: room.pushNotificationState === PushNotificationState.Default
                enabled: room.pushNotificationState != PushNotificationState.Unknown
                onToggled: {
                    room.pushNotificationState = PushNotificationState.Default
                }
            }
            QQC2.RadioButton {
                text: i18nc("As in 'notify for all messages'","All")
                checked: room.pushNotificationState === PushNotificationState.All
                enabled: room.pushNotificationState != PushNotificationState.Unknown
                onToggled: {
                    room.pushNotificationState = PushNotificationState.All
                }
            }
            QQC2.RadioButton {
                text: i18nc("As in 'notify when the user is mentioned or the message contains a set keyword'","@Mentions and Keywords")
                checked: room.pushNotificationState === PushNotificationState.MentionKeyword
                enabled: room.pushNotificationState != PushNotificationState.Unknown
                onToggled: {
                    room.pushNotificationState = PushNotificationState.MentionKeyword
                }
            }
            QQC2.RadioButton {
                text: i18nc("As in 'do not notify for any messages'","Off")
                checked: room.pushNotificationState === PushNotificationState.Mute
                enabled: room.pushNotificationState != PushNotificationState.Unknown
                onToggled: {
                    room.pushNotificationState = PushNotificationState.Mute
                }
            }
        }
    }
}
