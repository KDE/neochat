// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18nc("@title:window", "Appearance")
    leftPadding: 0
    rightPadding: 0
    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
            spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("General theme")
                }

                MobileForm.AbstractFormDelegate {
                    id: timelineModeSetting
                    Layout.fillWidth: true
                    background: Item {}
                    contentItem: RowLayout {
                        Layout.alignment: Qt.AlignCenter
                        spacing: Kirigami.Units.largeSpacing
                        Item {
                            Layout.fillWidth: true
                        }
                        QQC2.ButtonGroup { id: themeGroup }
                        ThemeRadioButton {
                            thin: timelineModeSetting.width < Kirigami.Units.gridUnit * 22
                            innerObject: [
                                RowLayout {
                                    Layout.fillWidth: true
                                    Kirigami.Avatar {
                                        color: "#4a5bcc"
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
                                                color: "#4a5bcc"
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
                                            border.width: 1
                                        }
                                    }
                                },
                                RowLayout {
                                    Layout.fillWidth: true
                                    Kirigami.Avatar {
                                        color: "#9f244b"
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
                                                color: "#9f244b"
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
                                            border.width: 1
                                        }
                                    }
                                }
                            ]

                            text: i18n("Bubbles")
                            checked: !Config.compactLayout
                            QQC2.ButtonGroup.group: themeGroup
                            enabled: !Config.isCompactLayoutImmutable

                            onToggled: {
                                Config.compactLayout = !checked;
                                Config.save();
                            }
                        }
                        ThemeRadioButton {
                            // Layout.alignment: Qt.AlignRight
                            thin: timelineModeSetting.width < Kirigami.Units.gridUnit * 22
                            innerObject: [
                                RowLayout {
                                    Layout.fillWidth: true
                                    Kirigami.Avatar {
                                        color: "#4a5bcc"
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
                                            color: "#4a5bcc"
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
                                        color: "#9f244b"
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
                                            color: "#9f244b"
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
                            enabled: !Config.isCompactLayoutImmutable

                            onToggled: {
                                Config.compactLayout = checked;
                                Config.save();
                            }
                        }
                        Item {
                            Layout.fillWidth: true
                        }
                    }
                }
                
                MobileForm.FormDelegateSeparator { below: colorSchemeDelegate.item ; visible: colorSchemeDelegate.visible }
                
                Loader {
                    id: colorSchemeDelegate
                    visible: item !== null && Qt.platform.os !== "android"
                    source: "qrc:/ColorScheme.qml"
                    Layout.fillWidth: true
                }
            }
        }
        
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCheckDelegate {
                    id: showFancyEffectsDelegate
                    text: i18n("Show fancy effects in chat")
                    checked: Config.showFancyEffects
                    enabled: !Config.isShowFancyEffectsImmutable
                    onToggled: {
                        Config.showFancyEffects = checked;
                        Config.save();
                    }
                }

                MobileForm.FormDelegateSeparator { above: showFancyEffectsDelegate ; below: hasWindowSystemDelegate }

                MobileForm.FormCheckDelegate {
                    id: hasWindowSystemDelegate
                    visible: Controller.hasWindowSystem
                    text: i18n("Use transparent chat page")
                    enabled: !Config.compactLayout && !Config.isBlurImmutable
                    checked: Config.blur
                    onToggled: {
                        Config.blur = checked;
                        Config.save();
                    }
                }

                MobileForm.FormDelegateSeparator { above: hasWindowSystemDelegate; below: transparencyDelegate }

                MobileForm.AbstractFormDelegate {
                    id: transparencyDelegate
                    Layout.fillWidth: true
                    visible: Controller.hasWindowSystem && Config.blur
                    enabled: !Config.isTransparancyImmutable
                    background: Item {}
                    contentItem: ColumnLayout {
                        QQC2.Label {
                            text: i18n("Transparency")
                            Layout.fillWidth: true
                        }
                        QQC2.Slider {
                            enabled: !Config.compactLayout && Config.blur
                            from: 0
                            to: 1
                            stepSize: 0.05
                            value: Config.transparency
                            onMoved: {
                                Config.transparency = value;
                                Config.save();
                            }
                            Layout.fillWidth: true

                            HoverHandler { id: sliderHover }
                            QQC2.ToolTip.visible: sliderHover.hovered && !enabled
                            QQC2.ToolTip.text: i18n("Only enabled if the transparent chat page is enabled.")
                        }
                        QQC2.Label {
                            text: Math.round(Config.transparency * 100) + "%"
                            Layout.fillWidth: true
                        }
                    }
                }

                MobileForm.FormDelegateSeparator { above: transparencyDelegate; below: showLocalMessagesOnRightDelegate; visible: transparencyDelegate.visible }

                MobileForm.FormCheckDelegate {
                    id: showLocalMessagesOnRightDelegate
                    text: i18n("Show your messages on the right")
                    checked: Config.showLocalMessagesOnRight
                    enabled: !Config.isShowLocalMessagesOnRightImmutable && !Config.compactLayout
                    onToggled: {
                        Config.showLocalMessagesOnRight = checked
                        Config.save()
                    }
                }

                MobileForm.FormDelegateSeparator { above: showLocalMessagesOnRightDelegate; below: showLinkPreviewDelegate }

                MobileForm.FormCheckDelegate {
                    id: showLinkPreviewDelegate
                    text: i18n("Show links preview in the chat messages")
                    checked: Config.showLinkPreview
                    onToggled: {
                        Config.showLinkPreview = checked
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
                    title: i18n("Show Avatar")
                }
                
                MobileForm.FormCheckDelegate {
                    text: i18n("In chat")
                    checked: Config.showAvatarInTimeline
                    onToggled: {
                        Config.showAvatarInTimeline = checked
                        Config.save()
                    }
                    enabled: !Config.isShowAvatarInTimelineImmutable
                }

                MobileForm.FormCheckDelegate {
                    text: i18n("In sidebar")
                    checked: Config.showAvatarInRoomDrawer
                    enabled: !Config.isShowAvatarInRoomDrawerImmutable
                    onToggled: {
                        Config.showAvatarInRoomDrawer = checked
                        Config.save()
                    }
                }
            }
        }
    }
}
