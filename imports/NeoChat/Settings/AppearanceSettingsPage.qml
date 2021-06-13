// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Settings 1.0

Kirigami.ScrollablePage {
    ColumnLayout {
        RowLayout {
            Layout.alignment: Qt.AlignCenter
            spacing: Kirigami.Units.gridUnit * 2
            QQC2.ButtonGroup { id: themeGroup }
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
                source: "qrc:/imports/NeoChat/Settings/ColorScheme.qml"
            }
            QQC2.CheckBox {
                visible: Controller.hasWindowSystem
                text: i18n("Use transparent chat page")
                enabled: !Config.compactLayout
                checked: Config.blur
                onToggled: {
                    Config.blur = checked;
                    Config.save();
                }
            }
            QQC2.CheckBox {
                text: i18n("Show your messages on the right")
                checked: Config.showLocalMessagesOnRight
                onToggled: {
                    Config.showLocalMessagesOnRight = checked
                    Config.save()
                }
            }
        }
    }
}
