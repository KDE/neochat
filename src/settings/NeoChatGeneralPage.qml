// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat
import org.kde.neochat.devtools

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title:window", "General")

    FormCard.FormHeader {
        title: i18nc("@title:group", "General Settings")
        visible: Qt.platform.os !== "android"
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            id: closeDelegate
            text: i18n("Show in System Tray")
            checked: NeoChatConfig.systemTray
            visible: Controller.supportSystemTray
            enabled: !NeoChatConfig.isSystemTrayImmutable
            onToggled: {
                NeoChatConfig.systemTray = checked;
                NeoChatConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: closeDelegate
            below: minimizeDelegate
        }

        FormCard.FormCheckDelegate {
            id: minimizeDelegate
            text: i18n("Minimize to system tray on startup")
            checked: NeoChatConfig.minimizeToSystemTrayOnStartup
            visible: Controller.supportSystemTray && !Kirigami.Settings.isMobile && NeoChatConfig.systemTray
            enabled: NeoChatConfig.systemTray && !NeoChatConfig.isMinimizeToSystemTrayOnStartupImmutable
            onToggled: {
                NeoChatConfig.minimizeToSystemTrayOnStartup = checked;
                NeoChatConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: minimizeDelegate
            below: automaticallyDelegate
            visible: minimizeDelegate.visible
        }

        FormCard.FormCheckDelegate {
            id: automaticallyDelegate
            text: i18n("Automatically hide/unhide the room information when resizing the window")
            checked: NeoChatConfig.autoRoomInfoDrawer
            enabled: !NeoChatConfig.isAutoRoomInfoDrawerImmutable
            visible: Qt.platform.os !== "android"
            onToggled: {
                NeoChatConfig.autoRoomInfoDrawer = checked;
                NeoChatConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: automaticallyDelegate
            below: categorizeDelegate
        }
        FormCard.FormCheckDelegate {
            id: categorizeDelegate
            text: i18n("Show all rooms in \"Home\" tab")
            checked: NeoChatConfig.allRoomsInHome
            enabled: !NeoChatConfig.isAllRoomsInHomeImmutable
            onToggled: {
                NeoChatConfig.allRoomsInHome = checked;
                NeoChatConfig.save();
            }
        }

    }
    FormCard.FormHeader {
        title: i18nc("@title:group", "Room List Sort Order")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18nc("@info:label", "Hidden events are not considered as recent activity when sorting rooms.")
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormRadioDelegate {
            text: i18nc("As in 'sort something based on last activity'", "Importance")
            description: i18nc("@info", "Rooms with unread notifications will be shown first.")
            checked: NeoChatConfig.sortOrder === 1
            enabled: !NeoChatConfig.isSortOrderImmutable
            onToggled: {
                NeoChatConfig.sortOrder = 1
                NeoChatConfig.customSortOrder = []
                NeoChatConfig.save()
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("As in 'sort something alphabetically'", "Alphabetical")
            checked: NeoChatConfig.sortOrder === 0
            enabled: !NeoChatConfig.isSortOrderImmutable
            onToggled: {
                NeoChatConfig.sortOrder = 0
                NeoChatConfig.customSortOrder = []
                NeoChatConfig.save()
            }
        }
        FormCard.FormRadioDelegate {
            text: i18nc("As in 'sort something based on the last event'", "Newest Events")
            description: i18nc("@info", "Rooms with the newest events will be shown first.")
            checked: NeoChatConfig.sortOrder === 2
            enabled: !NeoChatConfig.isSortOrderImmutable
            onToggled: {
                NeoChatConfig.sortOrder = 2
                NeoChatConfig.customSortOrder = []
                NeoChatConfig.save()
            }
        }
        FormCard.FormRadioDelegate {
            id: openCustomRoomSortButton
            text: i18nc("@option:radio", "Custom")
            checked: NeoChatConfig.sortOrder === 3
            enabled: !NeoChatConfig.isSortOrderImmutable
            onClicked: {
                Qt.createComponent('org.kde.neochat.settings', 'RoomSortParameterDialog').createObject(root).open();
            }
        }
    }
    FormCard.FormHeader {
        title: i18nc("@title", "Timeline")
    }
    FormCard.FormCard {
        FormCard.FormComboBoxDelegate {
            id: markAsReadCombo
            text: i18n("Mark messages as read when:")
            textRole: "name"
            valueRole: "value"
            model: [
                {
                    name: i18n("Never"),
                    value: 0
                },
                {
                    name: i18nc("@item:inlistbox As in mark messages in the room as read when entering the room", "Entering the room"),
                    value: 1
                },
                {
                    name: i18nc("@item:inlistbox As in mark messages in the room as read when entering the room and all messages are visible on screen", "Entering the room and all unread messages are visible"),
                    value: 2
                },
                {
                    name: i18nc("@item:inlistbox As in mark messages in the room as read when exiting the room", "Exiting the room"),
                    value: 3
                },
                {
                    name: i18nc("@item:inlistbox As in mark messages in the room as read when exiting the room and all messages are visible on screen", "Exiting the room and all unread messages are visible"),
                    value: 4
                }
            ]
            Component.onCompleted: currentIndex = NeoChatConfig.markReadCondition
            onCurrentValueChanged: NeoChatConfig.markReadCondition = currentValue
        }

        FormCard.FormDelegateSeparator {
            above: markAsReadCombo
            below: showDeletedMessages
        }

        FormCard.FormCheckDelegate {
            id: showDeletedMessages
            text: i18n("Show deleted messages")
            checked: NeoChatConfig.showDeletedMessages
            enabled: !NeoChatConfig.isShowDeletedMessagesImmutable
            onToggled: {
                NeoChatConfig.showDeletedMessages = checked;
                NeoChatConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: showDeletedMessages
            below: showStateEvents
        }

        FormCard.FormCheckDelegate {
            id: showStateEvents
            text: i18n("Show state events")
            checked: NeoChatConfig.showStateEvent
            enabled: !NeoChatConfig.isShowStateEventImmutable
            onToggled: {
                NeoChatConfig.showStateEvent = checked;
                NeoChatConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            visible: NeoChatConfig.showStateEvent
            above: showStateEvents
            below: showLeaveJoinEventDelegate
        }

        FormCard.FormCheckDelegate {
            id: showLeaveJoinEventDelegate
            visible: NeoChatConfig.showStateEvent
            text: i18n("Show leave and join events")
            checked: NeoChatConfig.showLeaveJoinEvent
            enabled: !NeoChatConfig.isShowLeaveJoinEventImmutable
            onToggled: {
                NeoChatConfig.showLeaveJoinEvent = checked;
                NeoChatConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            visible: NeoChatConfig.showStateEvent
            above: showLeaveJoinEventDelegate
            below: showNameDelegate
        }

        FormCard.FormCheckDelegate {
            id: showNameDelegate
            visible: NeoChatConfig.showStateEvent
            text: i18n("Show name change events")
            checked: NeoChatConfig.showRename
            enabled: !NeoChatConfig.isShowRenameImmutable
            onToggled: {
                NeoChatConfig.showRename = checked;
                NeoChatConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            visible: NeoChatConfig.showStateEvent
            above: showNameDelegate
            below: showAvatarChangeDelegate
        }

        FormCard.FormCheckDelegate {
            id: showAvatarChangeDelegate
            visible: NeoChatConfig.showStateEvent
            text: i18n("Show avatar update events")
            checked: NeoChatConfig.showAvatarUpdate
            enabled: !NeoChatConfig.isShowAvatarUpdateImmutable
            onToggled: {
                NeoChatConfig.showAvatarUpdate = checked;
                NeoChatConfig.save();
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
            checked: NeoChatConfig.allowQuickEdit
            enabled: !NeoChatConfig.isAllowQuickEditImmutable
            onToggled: {
                NeoChatConfig.allowQuickEdit = checked;
                NeoChatConfig.save();
            }
        }
        FormCard.FormDelegateSeparator {
            above: quickEditCheckbox
            below: typingNotificationsDelegate
        }
        FormCard.FormCheckDelegate {
            id: typingNotificationsDelegate
            text: i18n("Send typing notifications")
            checked: NeoChatConfig.typingNotifications
            enabled: !NeoChatConfig.isTypingNotificationsImmutable
            onToggled: {
                NeoChatConfig.typingNotifications = checked;
                NeoChatConfig.save();
            }
        }
    }
    FormCard.FormHeader {
        title: i18n("Developer Settings")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            id: enableDeveloperToolsDelegate
            text: i18n("Enable developer tools")
            checked: NeoChatConfig.developerTools
            enabled: !NeoChatConfig.isDeveloperToolsImmutable
            onToggled: {
                NeoChatConfig.developerTools = checked;
                NeoChatConfig.save();
            }
        }
        FormCard.FormDelegateSeparator {
            above: enableDeveloperToolsDelegate
            below: openDeveloperToolsDelegate
        }
        FormCard.FormButtonDelegate {
            id: openDeveloperToolsDelegate
            visible: NeoChatConfig.developerTools
            icon.name: "tools"
            text: i18n("Open Developer Tools")
            onClicked: root.QQC2.ApplicationWindow.window.pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.devtools', 'DevtoolsPage'), {
                connection: root.connection
            }, {
                title: i18n("Developer Tools")
            });
        }
    }
    FormCard.FormHeader {
        title: i18nc("@title:group", "Default Settings")
    }
    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            icon.name: "kt-restore-defaults-symbolic"
            text: i18nc("@action:button", "Reset all configuration values to their default…")
            onClicked: resetDialog.open()
        }
    }
    Kirigami.PromptDialog {
        id: resetDialog
        title: i18nc("@title:dialog", "Reset Configuration")
        subtitle: i18nc("@info", "Do you really want to reset all options to their default values?")
        standardButtons: Kirigami.Dialog.Ok | Kirigami.Dialog.Cancel
        onAccepted: Controller.revertToDefaultConfig()
        anchors.centerIn: QQC2.Overlay.overlay
    }
}
