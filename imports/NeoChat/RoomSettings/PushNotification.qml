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
                text: i18n("Default")
                Kirigami.FormData.label: i18n("Room notifications setting:")
                checked: room.pushNotificationState === PushNotificationState.Default
                enabled: room.pushNotificationState != PushNotificationState.Unknown
                onToggled: {
                    room.pushNotificationState = PushNotificationState.Default
                }
            }
            QQC2.RadioButton {
                text: i18n("All messages")
                checked: room.pushNotificationState === PushNotificationState.All
                enabled: room.pushNotificationState != PushNotificationState.Unknown
                onToggled: {
                    room.pushNotificationState = PushNotificationState.All
                }
            }
            QQC2.RadioButton {
                text: i18n("@mentions and keywords")
                checked: room.pushNotificationState === PushNotificationState.MentionKeyword
                enabled: room.pushNotificationState != PushNotificationState.Unknown
                onToggled: {
                    room.pushNotificationState = PushNotificationState.MentionKeyword
                }
            }
            QQC2.RadioButton {
                text: i18n("Off")
                checked: room.pushNotificationState === PushNotificationState.Mute
                enabled: room.pushNotificationState != PushNotificationState.Unknown
                onToggled: {
                    room.pushNotificationState = PushNotificationState.Mute
                }
            }
        }
    }
}
