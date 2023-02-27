// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18nc("@title:window", "General")
    leftPadding: 0
    rightPadding: 0
    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("General settings")
                    visible: Qt.platform.os !== "android"
                }

                MobileForm.FormCheckDelegate {
                    id: closeDelegate
                    text: i18n("Close to system tray")
                    checked: Config.systemTray
                    visible: Controller.supportSystemTray
                    enabled: !Config.isSystemTrayImmutable
                    onToggled: {
                        Config.systemTray = checked
                        Config.save()
                    }
                }

                MobileForm.FormDelegateSeparator { above: closeDelegate; below: minimizeDelegate }

                MobileForm.FormCheckDelegate {
                    id: minimizeDelegate
                    text: i18n("Minimize to system tray on startup")
                    checked: Config.minimizeToSystemTrayOnStartup
                    visible: Controller.supportSystemTray && !Kirigami.Settings.isMobile
                    enabled: Config.systemTray && !Config.isMinimizeToSystemTrayOnStartupImmutable
                    onToggled: {
                        Config.minimizeToSystemTrayOnStartup = checked
                        Config.save()
                    }
                }

                MobileForm.FormDelegateSeparator { above: minimizeDelegate; below: automaticallyDelegate }

                MobileForm.FormCheckDelegate {
                    id: automaticallyDelegate
                    text: i18n("Automatically hide/unhide the room information when resizing the window")
                    checked: Config.autoRoomInfoDrawer
                    enabled: !Config.isAutoRoomInfoDrawerImmutable
                    visible: Qt.platform.os !== "android"
                    onToggled: {
                        Config.autoRoomInfoDrawer = checked
                        Config.save()
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Timeline Events")
                }

                MobileForm.FormCheckDelegate {
                    id: showLeaveJoinEventDelegate
                    text: i18n("Show leave and join events")
                    checked: Config.showLeaveJoinEvent
                    enabled: !Config.isShowLeaveJoinEventImmutable
                    onToggled: {
                        Config.showLeaveJoinEvent = checked
                        Config.save()
                    }
                }

                MobileForm.FormDelegateSeparator { above: showLeaveJoinEventDelegate; below: showNameDelegate }

                MobileForm.FormCheckDelegate {
                    id: showNameDelegate
                    text: i18n("Show name change events")
                    checked: Config.showRename
                    enabled: !Config.isShowRenameImmutable
                    onToggled: {
                        Config.showRename = checked
                        Config.save()
                    }
                }

                MobileForm.FormDelegateSeparator { above: showNameDelegate; below: showAvatarChangeDelegate }

                MobileForm.FormCheckDelegate {
                    id: showAvatarChangeDelegate
                    text: i18n("Show avatar update events")
                    checked: Config.showAvatarUpdate
                    enabled: !Config.isShowAvatarUpdateImmutable
                    onToggled: {
                        Config.showAvatarUpdate = checked
                        Config.save()
                    }
                }

                MobileForm.FormDelegateSeparator { above: showAvatarChangeDelegate; below: showDeletedMessages }

                MobileForm.FormCheckDelegate {
                    id: showDeletedMessages
                    text: i18n("Show deleted messages")
                    checked: Config.showDeletedMessages
                    enabled: !Config.isShowDeletedMessagesImmutable
                    onToggled: {
                        Config.showDeletedMessages = checked
                        Config.save()
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Rooms and private chats")
                }
                MobileForm.FormRadioDelegate {
                    text: i18n("Separated")
                    checked: !Config.mergeRoomList
                    enabled: !Config.isMergeRoomListImmutable
                    onToggled: {
                        Config.mergeRoomList = false
                        Config.save()
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18n("Intermixed")
                    checked: Config.mergeRoomList
                    enabled: !Config.isMergeRoomListImmutable
                    onToggled: {
                        Config.mergeRoomList = true
                        Config.save()
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18nc("Chat Editor", "Editor")
                }
                MobileForm.FormCheckDelegate {
                    id: quickEditCheckbox
                    text: i18n("Use s/text/replacement syntax to edit your last message")
                    checked: Config.allowQuickEdit
                    enabled: !Config.isAllowQuickEditImmutable
                    onToggled: {
                        Config.allowQuickEdit = checked
                        Config.save()
                    }
                }
                MobileForm.FormDelegateSeparator { above: quickEditCheckbox; below: typingNotificationsDelegate }
                MobileForm.FormCheckDelegate {
                    id: typingNotificationsDelegate
                    text: i18n("Send typing notifications")
                    checked: Config.typingNotifications
                    enabled: !Config.isTypingNotificationsImmutable
                    onToggled: {
                        Config.typingNotifications = checked
                        Config.save()
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Developer Settings")
                }
                MobileForm.FormCheckDelegate {
                    text: i18n("Enable developer tools")
                    checked: Config.developerTools
                    enabled: !Config.isDeveloperToolsImmutable
                    onToggled: {
                        Config.developerTools = checked
                        Config.save()
                    }
                }
            }
        }
    }
}
