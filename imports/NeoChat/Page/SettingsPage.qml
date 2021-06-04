// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Settings 1.0

Kirigami.ScrollablePage {
    title: i18n("Settings")
    bottomPadding: 0
    leftPadding: 0
    rightPadding: 0
    topPadding: 0

    onBackRequested: {
        if (pageSettingStack.depth > 1 && !pageSettingStack.wideMode && pageSettingStack.currentIndex !== 0) {
            event.accepted = true;
            pageSettingStack.pop();
        }
    }

    Kirigami.PageRow {
        id: pageSettingStack
        anchors.fill: parent
        columnView.columnWidth: Kirigami.Units.gridUnit * 12
        initialPage: Kirigami.ScrollablePage {
            bottomPadding: 0
            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            ListView {
                Component.onCompleted: actions[0].trigger();
                property list<Kirigami.Action> actions: [
                    Kirigami.Action {
                        text: i18n("General")
                        icon.name: "org.kde.neochat"
                        onTriggered: pageSettingStack.push(generalSettings)
                    },
                    Kirigami.Action {
                        text: i18n("Appearance")
                        icon.name: "preferences-desktop-theme-global"
                        onTriggered: pageSettingStack.push(appearanceSettings)
                    }
                ]
                model: actions
                delegate: Kirigami.BasicListItem {
                    action: modelData
                }
            }
        }
    }

    Component {
        id: generalSettings
        Kirigami.ScrollablePage {
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
                    text: i18n("Use s/text/replacement syntax to edit your last message")
                    checked: Config.allowQuickEdit
                    onToggled: {
                        Config.allowQuickEdit = checked
                        Config.save()
                    }
                }
            }
        }
    }

    Component {
        id: appearanceSettings
        Kirigami.ScrollablePage {
            ColumnLayout {
                RowLayout {
                    Layout.alignment: Qt.AlignCenter
                    spacing: Kirigami.Units.gridUnit * 2
                    ThemeRadioButton {
                        innerObject: [
                            RowLayout {
                                Layout.fillWidth: true
                                Kirigami.Avatar {
                                    color: "blue"
                                    Layout.alignment: Qt.AlignTop
                                    visible: Config.showAvatarInTimeline
                                    Layout.preferredWidth: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing * 2 : 0
                                    Layout.preferredHeight: Kirigami.Units.largeSpacing * 2
                                }
                                QQC2.Control {
                                    Layout.fillWidth: true
                                    contentItem: ColumnLayout {
                                        QQC2.Label {
                                            Layout.fillWidth: true
                                            font.weight: Font.Bold
                                            font.pixelSize: 7
                                            text: "Paul Müller"
                                            color: "blue"
                                            wrapMode: Text.Wrap
                                        }
                                        QQC2.Label {
                                            Layout.fillWidth: true
                                            text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus facilisis porta mauris, quis finibus sem suscipit tincidunt."
                                            wrapMode: Text.Wrap
                                            font.pixelSize: 7
                                        }
                                    }
                                    background: Kirigami.ShadowedRectangle {
                                        color: Kirigami.Theme.backgroundColor
                                        radius: Kirigami.Units.smallSpacing
                                        shadow.size: Kirigami.Units.smallSpacing
                                        shadow.color: Qt.rgba(0.0, 0.0, 0.0, 0.10)
                                        border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                                        border.width: Kirigami.Units.devicePixelRatio
                                    }
                                }
                            },
                            RowLayout {
                                Layout.fillWidth: true
                                Kirigami.Avatar {
                                    color: "red"
                                    Layout.alignment: Qt.AlignTop
                                    visible: Config.showAvatarInTimeline
                                    Layout.preferredWidth: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing * 2 : 0
                                    Layout.preferredHeight: Kirigami.Units.largeSpacing * 2
                                }
                                QQC2.Control {
                                    Layout.fillWidth: true
                                    contentItem: ColumnLayout {
                                        QQC2.Label {
                                            Layout.fillWidth: true
                                            font.weight: Font.Bold
                                            font.pixelSize: 7
                                            text: "Jean Paul"
                                            color: "red"
                                            wrapMode: Text.Wrap
                                        }
                                        QQC2.Label {
                                            Layout.fillWidth: true
                                            text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus facilisis porta , quis sem suscipit tincidunt."
                                            wrapMode: Text.Wrap
                                            font.pixelSize: 7
                                        }
                                    }
                                    background: Kirigami.ShadowedRectangle {
                                        color: Kirigami.Theme.backgroundColor
                                        radius: Kirigami.Units.smallSpacing
                                        shadow.size: Kirigami.Units.smallSpacing
                                        shadow.color: Qt.rgba(0.0, 0.0, 0.0, 0.10)
                                        border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                                        border.width: Kirigami.Units.devicePixelRatio
                                    }
                                }
                            }
                        ]

                        text: i18n("Bubbles")
                        checked: !Config.compactLayout
                        QQC2.ButtonGroup.group: themeGroup

                        onToggled: {
                            Config.compactLayout = !checked;
                            Config.save();
                        }
                    }
                    ThemeRadioButton {
                        innerObject: [
                            RowLayout {
                                Layout.fillWidth: true
                                Kirigami.Avatar {
                                    color: "blue"
                                    Layout.alignment: Qt.AlignTop
                                    visible: Config.showAvatarInTimeline
                                    Layout.preferredWidth: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing * 2 : 0
                                    Layout.preferredHeight: Kirigami.Units.largeSpacing * 2
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    QQC2.Label {
                                        Layout.fillWidth: true
                                        font.weight: Font.Bold
                                        font.pixelSize: 7
                                        text: "Paul Müller"
                                        color: "blue"
                                        wrapMode: Text.Wrap
                                    }
                                    QQC2.Label {
                                        Layout.fillWidth: true
                                        text: "Lorem ipsum dolor sit amet, consectetur elit. Vivamus facilisis porta mauris, finibus sem suscipit tincidunt."
                                        wrapMode: Text.Wrap
                                        font.pixelSize: 7
                                    }
                                }
                            },
                            RowLayout {
                                Layout.fillWidth: true
                                Kirigami.Avatar {
                                    color: "red"
                                    Layout.alignment: Qt.AlignTop
                                    visible: Config.showAvatarInTimeline
                                    Layout.preferredWidth: Config.showAvatarInTimeline ? Kirigami.Units.largeSpacing * 2 : 0
                                    Layout.preferredHeight: Kirigami.Units.largeSpacing * 2
                                }
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    QQC2.Label {
                                        Layout.fillWidth: true
                                        font.weight: Font.Bold
                                        font.pixelSize: 7
                                        text: "Jean Paul"
                                        color: "red"
                                        wrapMode: Text.Wrap
                                    }
                                    QQC2.Label {
                                        Layout.fillWidth: true
                                        text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Vivamus facilisis porta mauris, quis finibus sem suscipit tincidunt."
                                        wrapMode: Text.Wrap
                                        font.pixelSize: 7
                                    }
                                }
                            }
                        ]
                        text: i18n("Compact")
                        checked: Config.compactLayout
                        QQC2.ButtonGroup.group: themeGroup

                        onToggled: {
                            Config.compactLayout = checked;
                            Config.save();
                        }
                    }
                }
                Kirigami.FormLayout {
                    QQC2.CheckBox {
                        text: i18n("Show User Avatar")
                        checked: Config.showAvatarInTimeline
                        onToggled: {
                            Config.showAvatarInTimeline = checked;
                            Config.save();
                        }
                    }

                    QQC2.CheckBox {
                        text: i18n("Show Fancy Effects")
                        checked: Config.showFancyEffects
                        onToggled: {
                            Config.showFancyEffects = checked;
                            Config.save();
                        }
                    }
                    Loader {
                        visible: item !== null
                        Kirigami.FormData.label: item ? i18n("Theme:") : ""
                        Kirigami.FormData.buddyFor: item ? item.slider : null
                        source: "qrc:/imports/NeoChat/Settings/ColorScheme.qml"
                    }
                }
            }
        }
    }
}
