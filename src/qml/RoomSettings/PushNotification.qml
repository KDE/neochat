// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {

    property NeoChatRoom room

    title: i18nc('@title:window', 'Notifications')

    leftPadding: 0
    rightPadding: 0

    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Room notifications setting")
                }
                MobileForm.FormRadioDelegate {
                    text: i18n("Follow global setting")
                    checked: room.pushNotificationState === PushNotificationState.Default
                    enabled: room.pushNotificationState !== PushNotificationState.Unknown
                    onToggled: {
                        room.pushNotificationState = PushNotificationState.Default
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("As in 'notify for all messages'","All")
                    checked: room.pushNotificationState === PushNotificationState.All
                    enabled: room.pushNotificationState !== PushNotificationState.Unknown
                    onToggled: {
                        room.pushNotificationState = PushNotificationState.All
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("As in 'notify when the user is mentioned or the message contains a set keyword'","@Mentions and Keywords")
                    checked: room.pushNotificationState === PushNotificationState.MentionKeyword
                    enabled: room.pushNotificationState !== PushNotificationState.Unknown
                    onToggled: {
                        room.pushNotificationState = PushNotificationState.MentionKeyword
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("As in 'do not notify for any messages'","Off")
                    checked: room.pushNotificationState === PushNotificationState.Mute
                    enabled: room.pushNotificationState !== PushNotificationState.Unknown
                    onToggled: {
                        room.pushNotificationState = PushNotificationState.Mute
                    }
                }
            }
        }
    }
}
