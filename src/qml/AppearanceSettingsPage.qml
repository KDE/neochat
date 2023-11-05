// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat
import org.kde.neochat.config

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "Appearance")

    FormCard.FormHeader {
        title: i18n("General theme")
    }
    FormCard.FormCard {
        FormCard.AbstractFormDelegate {
            id: timelineModeSetting
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
                            KirigamiComponents.Avatar {
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
                            KirigamiComponents.Avatar {
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
                            KirigamiComponents.Avatar {
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
                            KirigamiComponents.Avatar {
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

        FormCard.FormDelegateSeparator { below: compactRoomListDelegate }

        FormCard.FormCheckDelegate {
            id: compactRoomListDelegate
            text: i18n("Use compact room list")
            checked: Config.compactRoomList
            onToggled: {
                Config.compactRoomList = checked;
                Config.save();
            }
        }

        FormCard.FormDelegateSeparator { above: compactRoomListDelegate ; below: colorSchemeDelegate.item ; visible: colorSchemeDelegate.visible }

        Loader {
            id: colorSchemeDelegate
            visible: item !== null
            source: "qrc:/org/kde/neochat/qml/ColorScheme.qml"
            Layout.fillWidth: true
        }
    }

    FormCard.FormCard {
        Layout.topMargin: Kirigami.Units.largeSpacing
        FormCard.FormCheckDelegate {
            id: showFancyEffectsDelegate
            text: i18n("Show fancy effects in chat")
            checked: Config.showFancyEffects
            enabled: !Config.isShowFancyEffectsImmutable
            onToggled: {
                Config.showFancyEffects = checked;
                Config.save();
            }
        }

        FormCard.FormDelegateSeparator { above: showFancyEffectsDelegate ; below: hasWindowSystemDelegate }

        FormCard.FormCheckDelegate {
            id: hasWindowSystemDelegate
            visible: WindowController.hasWindowSystem
            text: i18n("Use transparent chat page")
            enabled: !Config.compactLayout && !Config.isBlurImmutable
            checked: Config.blur
            onToggled: {
                Config.blur = checked;
                Config.save();
            }
        }

        FormCard.FormDelegateSeparator { above: hasWindowSystemDelegate; below: transparencyDelegate }

        FormCard.AbstractFormDelegate {
            id: transparencyDelegate
            visible: WindowController.hasWindowSystem && Config.blur
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

        FormCard.FormDelegateSeparator { above: transparencyDelegate; below: showLocalMessagesOnRightDelegate; visible: transparencyDelegate.visible }

        FormCard.FormCheckDelegate {
            id: showLocalMessagesOnRightDelegate
            text: i18n("Show your messages on the right")
            checked: Config.showLocalMessagesOnRight
            enabled: !Config.isShowLocalMessagesOnRightImmutable && !Config.compactLayout
            onToggled: {
                Config.showLocalMessagesOnRight = checked
                Config.save()
            }
        }

        FormCard.FormDelegateSeparator { above: showLocalMessagesOnRightDelegate; below: showLinkPreviewDelegate }

        FormCard.FormCheckDelegate {
            id: showLinkPreviewDelegate
            text: i18n("Show links preview in the chat messages")
            checked: Config.showLinkPreview
            onToggled: {
                Config.showLinkPreview = checked
                Config.save()
            }
        }
    }


    FormCard.FormHeader {
        title: i18n("Show Avatar")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18n("In chat")
            checked: Config.showAvatarInTimeline
            onToggled: {
                Config.showAvatarInTimeline = checked
                Config.save()
            }
            enabled: !Config.isShowAvatarInTimelineImmutable
        }

        FormCard.FormCheckDelegate {
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
