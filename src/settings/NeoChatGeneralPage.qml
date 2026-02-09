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

Kirigami.ScrollablePage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title:window", "General")

    Kirigami.Form {
        Kirigami.FormGroup {
            title: Qt.platform.os === "android" ? "" : i18nc("@title:group", "General Settings")

            Kirigami.FormEntry {
                visible: Controller.supportSystemTray
                contentItem: QQC2.CheckBox {
                    text: i18n("Show in System Tray")
                    checked: NeoChatConfig.systemTray
                    enabled: !NeoChatConfig.isSystemTrayImmutable
                    onToggled: {
                        NeoChatConfig.systemTray = checked;
                        NeoChatConfig.save();
                    }
                }
            }
            Kirigami.FormSeparator {}
            Kirigami.FormEntry {
                id: minimizeDelegate
                visible: Controller.supportSystemTray && !Kirigami.Settings.isMobile && NeoChatConfig.systemTray
                contentItem: QQC2.CheckBox {
                    text: i18n("Minimize to system tray on startup")
                    checked: NeoChatConfig.minimizeToSystemTrayOnStartup
                    enabled: NeoChatConfig.systemTray && !NeoChatConfig.isMinimizeToSystemTrayOnStartupImmutable
                    onToggled: {
                        NeoChatConfig.minimizeToSystemTrayOnStartup = checked;
                        NeoChatConfig.save();
                    }
                }
            }
            Kirigami.FormSeparator {
                visible: minimizeDelegate.visible
            }
            Kirigami.FormEntry {
                id: automaticallyDelegate
                visible: Qt.platform.os !== "android"
                contentItem: QQC2.CheckBox {
                    text: i18n("Automatically hide/unhide the room information when resizing the window")
                    checked: NeoChatConfig.autoRoomInfoDrawer
                    enabled: !NeoChatConfig.isAutoRoomInfoDrawerImmutable
                    onToggled: {
                        NeoChatConfig.autoRoomInfoDrawer = checked;
                        NeoChatConfig.save();
                    }
                }
            }
            Kirigami.FormSeparator {}
            Kirigami.FormEntry {
                id: categorizeDelegate
                contentItem: QQC2.CheckBox {
                    text: i18n("Show all rooms in \"Home\" tab")
                    checked: NeoChatConfig.allRoomsInHome
                    enabled: !NeoChatConfig.isAllRoomsInHomeImmutable
                    onToggled: {
                        NeoChatConfig.allRoomsInHome = checked;
                        NeoChatConfig.save();
                    }
                }
            }
        }
        Kirigami.FormGroup {
            title: i18nc("@title:group", "Room List Sort Order")
            Kirigami.FormEntry {
                title: i18nc("@info:label", "Hidden events are not considered as recent activity when sorting rooms.")
            }
            Kirigami.FormSeparator {}
            Kirigami.FormEntry {
                subtitle: i18nc("@info", "Rooms with unread notifications will be shown first.")
                contentItem: QQC2.RadioButton {
                    text: i18nc("As in 'sort something based on last activity'", "Importance")
                    checked: NeoChatConfig.sortOrder === 1
                    enabled: !NeoChatConfig.isSortOrderImmutable
                    onToggled: {
                        NeoChatConfig.sortOrder = 1
                        NeoChatConfig.customSortOrder = []
                        NeoChatConfig.save()
                    }
                }
            }
            Kirigami.FormEntry {
                contentItem: QQC2.RadioButton {
                    text: i18nc("As in 'sort something alphabetically'", "Alphabetical")
                    checked: NeoChatConfig.sortOrder === 0
                    enabled: !NeoChatConfig.isSortOrderImmutable
                    onToggled: {
                        NeoChatConfig.sortOrder = 0
                        NeoChatConfig.customSortOrder = []
                        NeoChatConfig.save()
                    }
                }
            }
            Kirigami.FormEntry {
                subtitle: i18nc("@info", "Rooms with the newest events will be shown first.")
                contentItem: QQC2.RadioButton {
                    text: i18nc("As in 'sort something based on the last event'", "Newest Events")
                    checked: NeoChatConfig.sortOrder === 2
                    enabled: !NeoChatConfig.isSortOrderImmutable
                    onToggled: {
                        NeoChatConfig.sortOrder = 2
                        NeoChatConfig.customSortOrder = []
                        NeoChatConfig.save()
                    }
                }
            }
            Kirigami.FormEntry {
                id: openCustomRoomSortButton
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio", "Custom")
                    checked: NeoChatConfig.sortOrder === 3
                    enabled: !NeoChatConfig.isSortOrderImmutable
                    onClicked: {
                        Qt.createComponent('org.kde.neochat.settings', 'RoomSortParameterDialog').createObject(root).open();
                    }
                }
            }
        }
        Kirigami.FormGroup {
            title: i18nc("@title", "Timeline")

            Kirigami.FormEntry {
                id: markAsReadCombo
                title: i18n("Mark messages as read when:")
                contentItem: QQC2.ComboBox {
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
            }
            Kirigami.FormSeparator {}
            Kirigami.FormEntry {
                id: showDeletedMessages
                contentItem: QQC2.CheckBox {
                    text: i18n("Show deleted messages")
                    checked: NeoChatConfig.showDeletedMessages
                    enabled: !NeoChatConfig.isShowDeletedMessagesImmutable
                    onToggled: {
                        NeoChatConfig.showDeletedMessages = checked;
                        NeoChatConfig.save();
                    }
                }
            }
            Kirigami.FormSeparator {}
            Kirigami.FormEntry {
                id: showStateEvents
                contentItem: QQC2.CheckBox {
                    text: i18n("Show state events")
                    checked: NeoChatConfig.showStateEvent
                    enabled: !NeoChatConfig.isShowStateEventImmutable
                    onToggled: {
                        NeoChatConfig.showStateEvent = checked;
                        NeoChatConfig.save();
                    }
                }
            }
            Kirigami.FormSeparator {
                visible: NeoChatConfig.showStateEvent
            }
            Kirigami.FormEntry {
                id: showLeaveJoinEventDelegate
                visible: NeoChatConfig.showStateEvent
                contentItem: QQC2.CheckBox {
                    text: i18n("Show leave and join events")
                    checked: NeoChatConfig.showLeaveJoinEvent
                    enabled: !NeoChatConfig.isShowLeaveJoinEventImmutable
                    onToggled: {
                        NeoChatConfig.showLeaveJoinEvent = checked;
                        NeoChatConfig.save();
                    }
                }
            }
            Kirigami.FormSeparator {
                visible: NeoChatConfig.showStateEvent
            }
            Kirigami.FormEntry {
                id: showNameDelegate
                visible: NeoChatConfig.showStateEvent
                contentItem: QQC2.CheckBox {
                    text: i18n("Show name change events")
                    checked: NeoChatConfig.showRename
                    enabled: !NeoChatConfig.isShowRenameImmutable
                    onToggled: {
                        NeoChatConfig.showRename = checked;
                        NeoChatConfig.save();
                    }
                }
            }
            Kirigami.FormSeparator {
                visible: NeoChatConfig.showStateEvent
            }
            Kirigami.FormEntry {
                id: showAvatarChangeDelegate
                visible: NeoChatConfig.showStateEvent
                contentItem: QQC2.CheckBox {
                    text: i18n("Show name change events")
                    checked: NeoChatConfig.showAvatarUpdate
                    enabled: !NeoChatConfig.isShowAvatarUpdateImmutable
                    onToggled: {
                        NeoChatConfig.showAvatarUpdate = checked;
                        NeoChatConfig.save();
                    }
                }
            }
        }
        Kirigami.FormGroup {
            title: i18nc("Chat Editor", "Editor")

            Kirigami.FormEntry {
                visible: !Kirigami.Settings.isMobile
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio", "Send messages with Enter")
                    checked: NeoChatConfig.sendMessageWith === 0
                    enabled: !NeoChatConfig.isSendMessageWithImmutable
                    onToggled: {
                        NeoChatConfig.sendMessageWith = 0
                        NeoChatConfig.save()
                    }
                }
            }
            Kirigami.FormEntry {
                visible: !Kirigami.Settings.isMobile
                contentItem: QQC2.RadioButton {
                    text: i18nc("@option:radio", "Send messages with Ctrl+Enter")
                    checked: NeoChatConfig.sendMessageWith === 1
                    enabled: !NeoChatConfig.isSendMessageWithImmutable
                    onToggled: {
                        NeoChatConfig.sendMessageWith = 1
                        NeoChatConfig.save()
                    }
                }
            }
            Kirigami.FormSeparator {
                visible: !Kirigami.Settings.isMobile
            }
            Kirigami.FormEntry {
                id: quickEditCheckbox
                contentItem: QQC2.CheckBox {
                    text: i18n("Use s/text/replacement syntax to edit your last message")
                    checked: NeoChatConfig.allowQuickEdit
                    enabled: !NeoChatConfig.isAllowQuickEditImmutable
                    onToggled: {
                        NeoChatConfig.allowQuickEdit = checked;
                        NeoChatConfig.save();
                    }
                }
            }
            Kirigami.FormSeparator {}
            Kirigami.FormEntry {
                id: typingNotificationsDelegate
                contentItem: QQC2.CheckBox {
                    text: i18n("Send typing notifications")
                    checked: NeoChatConfig.typingNotifications
                    enabled: !NeoChatConfig.isTypingNotificationsImmutable
                    onToggled: {
                        NeoChatConfig.typingNotifications = checked;
                        NeoChatConfig.save();
                    }
                }
            }
        }
        Kirigami.FormGroup {
            title: i18n("Developer Settings")

            Kirigami.FormEntry {
                id: enableDeveloperToolsDelegate
                contentItem: QQC2.CheckBox {
                    text: i18n("Enable developer tools")
                    checked: NeoChatConfig.developerTools
                    enabled: !NeoChatConfig.isDeveloperToolsImmutable
                    onToggled: {
                        NeoChatConfig.developerTools = checked;
                        NeoChatConfig.save();
                    }
                }
            }
            Kirigami.FormSeparator {}
            Kirigami.FormEntry {
                id: openDeveloperToolsDelegate
                visible: NeoChatConfig.developerTools
                leadingItems: Kirigami.Icon {
                    implicitWidth: Kirigami.Units.iconSizes.smallMedium
                    implicitHeight: Kirigami.Units.iconSizes.smallMedium
                    source: "tools"
                }
                contentItem: QQC2.Label {
                    text: i18n("Enable developer tools")
                }
                onClicked: root.QQC2.ApplicationWindow.window.pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat.devtools', 'DevtoolsPage'), {
                    connection: root.connection
                }, {
                    title: i18n("Developer Tools")
                });
            }
        }
        Kirigami.FormGroup {
            title: i18nc("@title:group", "Default Settings")

            Kirigami.FormEntry {
                leadingItems: Kirigami.Icon {
                    implicitWidth: Kirigami.Units.iconSizes.smallMedium
                    implicitHeight: Kirigami.Units.iconSizes.smallMedium
                    source: "kt-restore-defaults-symbolic"
                }
                contentItem: QQC2.Label {
                    text: i18nc("@action:button", "Reset all configuration values to their default…")
                }
                onClicked: resetDialog.open()
            }
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
