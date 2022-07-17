// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18nc('@title:window', 'General')
    ColumnLayout {
        Kirigami.FormLayout {
            Layout.fillWidth: true
            QQC2.CheckBox {
                Kirigami.FormData.label: i18n("General settings:")
                text: i18n("Close to system tray")
                checked: Config.systemTray
                visible: Controller.supportSystemTray
                enabled: !Config.isSystemTrayImmutable
                onToggled: {
                    Config.systemTray = checked
                    Config.save()
                }
            }
            QQC2.CheckBox {
                text: i18n("Minimize to system tray on startup")
                checked: Config.minimizeToSystemTrayOnStartup
                visible: Controller.supportSystemTray && !Kirigami.Settings.isMobile
                enabled: Config.systemTray && !Config.isMinimizeToSystemTrayOnStartupImmutable
                onToggled: {
                    Config.minimizeToSystemTrayOnStartup = checked
                    Config.save()
                }
            }
            QQC2.CheckBox {
                // TODO: When there are enough notification and timeline event
                // settings, make 2 separate groups with FormData labels.
                Kirigami.FormData.label: i18n("Notifications and events:")
                text: i18n("Show notifications")
                checked: Config.showNotifications
                enabled: !Config.isShowNotificationsImmutable
                onToggled: {
                    Config.showNotifications = checked
                    Config.save()
                }
            }
            QQC2.CheckBox {
                text: i18n("Show leave and join events")
                checked: Config.showLeaveJoinEvent
                enabled: !Config.isShowLeaveJoinEventImmutable
                onToggled: {
                    Config.showLeaveJoinEvent = checked
                    Config.save()
                }
            }
            QQC2.CheckBox {
                text: i18n("Show name change events")
                checked: Config.showRename
                enabled: !Config.isShowRenameImmutable
                onToggled: {
                    Config.showRename = checked
                    Config.save()
                }
            }
            QQC2.CheckBox {
                text: i18n("Show avatar update events")
                checked: Config.showAvatarUpdate
                enabled: !Config.isShowAvatarUpdateImmutable
                onToggled: {
                    Config.showAvatarUpdate = checked
                    Config.save()
                }
            }
            QQC2.RadioButton {
                Kirigami.FormData.label: i18n("Rooms and private chats:")
                text: i18n("Separated")
                checked: !Config.mergeRoomList
                enabled: !Config.isMergeRoomListImmutable
                onToggled: {
                    Config.mergeRoomList = false
                    Config.save()
                }
            }
            QQC2.RadioButton {
                text: i18n("Intermixed")
                checked: Config.mergeRoomList
                enabled: !Config.isMergeRoomListImmutable
                onToggled: {
                    Config.mergeRoomList = true
                    Config.save()
                }
            }
            QQC2.CheckBox {
                id: quickEditCheckbox
                Layout.maximumWidth: parent.width
                text: i18n("Use s/text/replacement syntax to edit your last message")
                checked: Config.allowQuickEdit
                enabled: !Config.isAllowQuickEditImmutable
                onToggled: {
                    Config.allowQuickEdit = checked
                    Config.save()
                }

                // TODO KF5.97 remove this line
                Component.onCompleted: this.contentItem.wrap = QQC2.Label.Wrap
            }
            QQC2.CheckBox {
                text: i18n("Send Typing Notifications")
                checked: Config.typingNotifications
                enabled: !Config.isTypingNotificationsImmutable
                onToggled: {
                    Config.typingNotifications = checked
                    Config.save()
                }
            }
            QQC2.CheckBox {
                text: i18n("Automatically hide/unhide the room information when resizing the window")
                Layout.maximumWidth: parent.width
                checked: Config.autoRoomInfoDrawer
                enabled: !Config.isAutoRoomInfoDrawerImmutable
                onToggled: {
                    Config.autoRoomInfoDrawer = checked
                    Config.save()
                }

                // TODO KF5.97 remove this line
                Component.onCompleted: this.contentItem.wrap = QQC2.Label.Wrap
            }
        }
    }
}
