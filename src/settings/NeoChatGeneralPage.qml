// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat
import org.kde.neochat.devtools

FormCard.FormCardPage {
    title: i18nc("@title:window", "General")

    FormCard.FormHeader {
        title: i18n("General settings")
        visible: Qt.platform.os !== "android"
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            id: closeDelegate
            text: i18n("Show in System Tray")
            checked: Config.systemTray
            visible: Controller.supportSystemTray
            enabled: !Config.isSystemTrayImmutable
            onToggled: {
                Config.systemTray = checked;
                Config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: closeDelegate
            below: minimizeDelegate
        }

        FormCard.FormCheckDelegate {
            id: minimizeDelegate
            text: i18n("Minimize to system tray on startup")
            checked: Config.minimizeToSystemTrayOnStartup
            visible: Controller.supportSystemTray && !Kirigami.Settings.isMobile
            enabled: Config.systemTray && !Config.isMinimizeToSystemTrayOnStartupImmutable
            onToggled: {
                Config.minimizeToSystemTrayOnStartup = checked;
                Config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: minimizeDelegate
            below: automaticallyDelegate
        }

        FormCard.FormCheckDelegate {
            id: automaticallyDelegate
            text: i18n("Automatically hide/unhide the room information when resizing the window")
            checked: Config.autoRoomInfoDrawer
            enabled: !Config.isAutoRoomInfoDrawerImmutable
            visible: Qt.platform.os !== "android"
            onToggled: {
                Config.autoRoomInfoDrawer = checked;
                Config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: automaticallyDelegate
            below: categorizeDelegate
        }
        FormCard.FormCheckDelegate {
            id: categorizeDelegate
            text: i18n("Show all rooms in \"Home\" tab")
            checked: Config.allRoomsInHome
            enabled: !Config.isAllRoomsInHomeImmutable
            onToggled: {
                Config.allRoomsInHome = checked;
                Config.save();
            }
        }

    }
    FormCard.FormHeader {
        title: i18n("Room list sort order")
    }
    FormCard.FormCard {
        FormCard.FormRadioDelegate {
            text: i18nc("As in 'sort something based on last activity'", "Activity")
            checked: Config.sortOrder === 1
            enabled: !Config.isSortOrderImmutable
            onToggled: {
                Config.sortOrder = 1
                Config.save()
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("As in 'sort something alphabetically'", "Alphabetical")
            checked: Config.sortOrder === 0
            enabled: !Config.isSortOrderImmutable
            onToggled: {
                Config.sortOrder = 0
                Config.save()
            }
        }
    }
    FormCard.FormHeader {
        title: i18n("Timeline Events")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            id: showDeletedMessages
            text: i18n("Show deleted messages")
            checked: Config.showDeletedMessages
            enabled: !Config.isShowDeletedMessagesImmutable
            onToggled: {
                Config.showDeletedMessages = checked;
                Config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: showDeletedMessages
            below: showStateEvents
        }

        FormCard.FormCheckDelegate {
            id: showStateEvents
            text: i18n("Show state events")
            checked: Config.showStateEvent
            enabled: !Config.isShowStateEventImmutable
            onToggled: {
                Config.showStateEvent = checked;
                Config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            visible: Config.showStateEvent
            above: showStateEvents
            below: showLeaveJoinEventDelegate
        }

        FormCard.FormCheckDelegate {
            id: showLeaveJoinEventDelegate
            visible: Config.showStateEvent
            text: i18n("Show leave and join events")
            checked: Config.showLeaveJoinEvent
            enabled: !Config.isShowLeaveJoinEventImmutable
            onToggled: {
                Config.showLeaveJoinEvent = checked;
                Config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            visible: Config.showStateEvent
            above: showLeaveJoinEventDelegate
            below: showNameDelegate
        }

        FormCard.FormCheckDelegate {
            id: showNameDelegate
            visible: Config.showStateEvent
            text: i18n("Show name change events")
            checked: Config.showRename
            enabled: !Config.isShowRenameImmutable
            onToggled: {
                Config.showRename = checked;
                Config.save();
            }
        }

        FormCard.FormDelegateSeparator {
            visible: Config.showStateEvent
            above: showNameDelegate
            below: showAvatarChangeDelegate
        }

        FormCard.FormCheckDelegate {
            id: showAvatarChangeDelegate
            visible: Config.showStateEvent
            text: i18n("Show avatar update events")
            checked: Config.showAvatarUpdate
            enabled: !Config.isShowAvatarUpdateImmutable
            onToggled: {
                Config.showAvatarUpdate = checked;
                Config.save();
            }
        }
    }
    FormCard.FormHeader {
        title: i18nc("Chat Editor", "Editor")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            id: quickEditCheckbox
            text: i18n("Use s/text/replacement syntax to edit your last message")
            checked: Config.allowQuickEdit
            enabled: !Config.isAllowQuickEditImmutable
            onToggled: {
                Config.allowQuickEdit = checked;
                Config.save();
            }
        }
        FormCard.FormDelegateSeparator {
            above: quickEditCheckbox
            below: typingNotificationsDelegate
        }
        FormCard.FormCheckDelegate {
            id: typingNotificationsDelegate
            text: i18n("Send typing notifications")
            checked: Config.typingNotifications
            enabled: !Config.isTypingNotificationsImmutable
            onToggled: {
                Config.typingNotifications = checked;
                Config.save();
            }
        }
    }
    FormCard.FormHeader {
        title: i18n("Developer Settings")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18n("Enable developer tools")
            checked: Config.developerTools
            enabled: !Config.isDeveloperToolsImmutable
            onToggled: {
                Config.developerTools = checked;
                Config.save();
            }
        }
        FormCard.FormButtonDelegate {
            visible: Config.developerTools
            text: i18n("Open developer tools")
            onClicked: applicationWindow().pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.devtools', 'DevtoolsPage'), {
                connection: root.connection
            }, {
                title: i18n("Developer Tools")
            });
        }
    }
}
