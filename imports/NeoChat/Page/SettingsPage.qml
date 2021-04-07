// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18n("Settings")

    Kirigami.FormLayout {
        QQC2.CheckBox {
            Kirigami.FormData.label: i18n("General settings:")
            text: i18n("Close to system tray")
            checked: Config.systemTray
            visible: Controller.supportSystemTray
            onToggled: {
                Config.systemTray = checked
                Config.save()
            }
        }
        QQC2.CheckBox {
            // TODO: When there are enough notification and timeline event
            // settings, make 2 separate groups with FormData labels.
            Kirigami.FormData.label: i18n("Notifications and events:")
            text: i18n("Show notifications")
            checked: Config.showNotifications
            onToggled: {
                Config.showNotifications = checked
                Config.save()
            }
        }
        QQC2.CheckBox {
            text: i18n("Show leave and join events")
            checked: Config.showLeaveJoinEvent
            onToggled: {
                Config.showLeaveJoinEvent = checked
                Config.save()
            }
        }
        QQC2.RadioButton {
            Kirigami.FormData.label: i18n("Rooms and private chats:")
            text: i18n("Separated")
            checked: !Config.mergeRoomList
            onToggled: {
                Config.mergeRoomList = false
                Config.save()
            }
        }
        QQC2.RadioButton {
            text: i18n("Intermixed")
            checked: Config.mergeRoomList
            onToggled: {
                Config.mergeRoomList = true
                Config.save()
            }
        }
        QQC2.CheckBox {
            Kirigami.FormData.label: i18n("Timeline:")
            text: i18n("Show User Avatar")
            checked: Config.showAvatarInTimeline
            onToggled: {
                Config.showAvatarInTimeline = checked
                Config.save()
            }
        }
        QQC2.CheckBox {
            text: i18n("Show Fancy Effects")
            checked: Config.showFancyEffects
            onToggled: {
                Config.showFancyEffects = checked
                Config.save()
            }
        }
    }
}
