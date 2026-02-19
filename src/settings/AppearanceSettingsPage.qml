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

FormCard.FormCardPage {
    id: root

    title: i18nc("@title:window", "Appearance")

    FormCard.FormHeader {
        title: i18nc("@title:group", "General")
    }
    FormCard.FormCard {
        Loader {
            id: colorSchemeDelegate
            visible: item !== null
            sourceComponent: Qt.createComponent('org.kde.neochat.settings', 'ColorScheme')
            Layout.fillWidth: true
        }

        FormCard.FormDelegateSeparator {
            above: colorSchemeDelegate
            below: compactRoomListDelegate
            visible: colorSchemeDelegate.visible
        }

        FormCard.FormCheckDelegate {
            id: compactRoomListDelegate
            text: i18n("Use compact room list")
            checked: NeoChatConfig.compactRoomList
            onToggled: {
                NeoChatConfig.compactRoomList = checked;
                NeoChatConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: fontScaleSliderDelegate
            below: compactRoomListDelegate
        }

        /*!
        Font scale setting allows user to adjust the font size used in the app.
        */
        FormCard.AbstractFormDelegate {
            id: fontScaleSliderDelegate
            background: Item {}
            contentItem: ColumnLayout {
                QQC2.Label {
                    text: i18nc("@label Font size for text in the chat pane", "Chat font scaling")
                    Layout.fillWidth: true
                }
                QQC2.Label {
                    text: i18nc("@label:slider Current font scale percentage. %1 is the numeric percentage value, the second % is the symbol e.g. 120%", "%1%", Math.round(NeoChatConfig.fontScale * 100))
                    Layout.fillWidth: true
                }
                QQC2.Slider {
                    from: 0.5
                    to: 3.0
                    stepSize: 0.1
                    value: NeoChatConfig.fontScale
                    onMoved: {
                        NeoChatConfig.fontScale = value;
                        NeoChatConfig.save();
                    }
                    Layout.fillWidth: true
                }
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Message Layout")
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
                QQC2.ButtonGroup {
                    id: themeGroup
                }
                ThemeRadioButton {
                    thin: timelineModeSetting.width < Kirigami.Units.gridUnit * 22
                    innerObject: [
                        RowLayout {
                            Layout.fillWidth: true
                            KirigamiComponents.Avatar {
                                color: "#4a5bcc"
                                Layout.alignment: Qt.AlignTop
                                visible: NeoChatConfig.showAvatarInTimeline
                                Layout.preferredWidth: NeoChatConfig.showAvatarInTimeline ? Kirigami.Units.largeSpacing * 2 : 0
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
                                    radius: Kirigami.Units.cornerRadius
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
                                visible: NeoChatConfig.showAvatarInTimeline
                                Layout.preferredWidth: NeoChatConfig.showAvatarInTimeline ? Kirigami.Units.largeSpacing * 2 : 0
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
                                    radius: Kirigami.Units.cornerRadius
                                    shadow.size: Kirigami.Units.smallSpacing
                                    shadow.color: Qt.rgba(0.0, 0.0, 0.0, 0.10)
                                    border.color: Kirigami.ColorUtils.tintWithAlpha(color, Kirigami.Theme.textColor, 0.15)
                                    border.width: 1
                                }
                            }
                        }
                    ]

                    text: i18n("Bubbles")
                    checked: !NeoChatConfig.compactLayout
                    QQC2.ButtonGroup.group: themeGroup
                    enabled: !NeoChatConfig.isCompactLayoutImmutable

                    onToggled: {
                        NeoChatConfig.compactLayout = !checked;
                        NeoChatConfig.save();
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
                                visible: NeoChatConfig.showAvatarInTimeline
                                Layout.preferredWidth: NeoChatConfig.showAvatarInTimeline ? Kirigami.Units.largeSpacing * 2 : 0
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
                                visible: NeoChatConfig.showAvatarInTimeline
                                Layout.preferredWidth: NeoChatConfig.showAvatarInTimeline ? Kirigami.Units.largeSpacing * 2 : 0
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
                    checked: NeoChatConfig.compactLayout
                    QQC2.ButtonGroup.group: themeGroup
                    enabled: !NeoChatConfig.isCompactLayoutImmutable

                    onToggled: {
                        NeoChatConfig.compactLayout = checked;
                        NeoChatConfig.save();
                    }
                }
                Item {
                    Layout.fillWidth: true
                }
            }
        }

        FormCard.FormDelegateSeparator {
            above: timelineModeSetting
            below: hasWindowSystemDelegate
        }

        FormCard.FormCheckDelegate {
            id: hasWindowSystemDelegate
            text: i18n("Use transparent chat page")
            enabled: !NeoChatConfig.compactLayout && !NeoChatConfig.isBlurImmutable
            visible: WindowController.hasWindowSystem && !NeoChatConfig.compactLayout
            checked: NeoChatConfig.blur
            onToggled: {
                NeoChatConfig.blur = checked;
                NeoChatConfig.save();
            }
        }

        FormCard.FormDelegateSeparator {
            above: hasWindowSystemDelegate
            below: transparencyDelegate
            visible: transparencyDelegate.visible
        }

        FormCard.AbstractFormDelegate {
            id: transparencyDelegate
            visible: WindowController.hasWindowSystem && NeoChatConfig.blur
            enabled: !NeoChatConfig.isTransparancyImmutable
            background: Item {}
            contentItem: ColumnLayout {
                QQC2.Label {
                    text: i18n("Transparency")
                    Layout.fillWidth: true
                }
                QQC2.Slider {
                    enabled: !NeoChatConfig.compactLayout && NeoChatConfig.blur
                    from: 0
                    to: 1
                    stepSize: 0.05
                    value: NeoChatConfig.transparency
                    onMoved: {
                        NeoChatConfig.transparency = value;
                        NeoChatConfig.save();
                    }
                    Layout.fillWidth: true

                    HoverHandler {
                        id: sliderHover
                    }
                    QQC2.ToolTip.visible: sliderHover.hovered && !enabled
                    QQC2.ToolTip.text: i18n("Only enabled if the transparent chat page is enabled.")
                }
                QQC2.Label {
                    text: Math.round(NeoChatConfig.transparency * 100) + "%"
                    Layout.fillWidth: true
                }
            }
        }

        FormCard.FormDelegateSeparator {
            above: transparencyDelegate
            below: showLocalMessagesOnRightDelegate
        }

        FormCard.FormCheckDelegate {
            id: showLocalMessagesOnRightDelegate
            text: i18n("Show your messages on the right")
            checked: NeoChatConfig.showLocalMessagesOnRight
            enabled: !NeoChatConfig.isShowLocalMessagesOnRightImmutable && !NeoChatConfig.compactLayout
            visible: !NeoChatConfig.compactLayout
            onToggled: {
                NeoChatConfig.showLocalMessagesOnRight = checked;
                NeoChatConfig.save();
            }
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Show Avatars")
    }
    FormCard.FormCard {
        FormCard.FormCheckDelegate {
            text: i18n("In chat")
            checked: NeoChatConfig.showAvatarInTimeline
            onToggled: {
                NeoChatConfig.showAvatarInTimeline = checked;
                NeoChatConfig.save();
            }
            enabled: !NeoChatConfig.isShowAvatarInTimelineImmutable
        }

        FormCard.FormCheckDelegate {
            text: i18n("In sidebar")
            checked: NeoChatConfig.showAvatarInRoomDrawer
            enabled: !NeoChatConfig.isShowAvatarInRoomDrawerImmutable
            onToggled: {
                NeoChatConfig.showAvatarInRoomDrawer = checked;
                NeoChatConfig.save();
            }
        }
    }
}
