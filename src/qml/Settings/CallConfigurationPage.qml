// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18nc("@title:window", "Calls")

    leftPadding: 0
    rightPadding: 0
    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Incoming Calls")
                }
                MobileForm.FormCheckDelegate {
                    text: i18n("Ring")
                    checked: Config.ring // TODO
                    enabled: !Config.isRingImmutable //TODO
                    onToggled: {
                        Config.ring = checked
                        Config.sync()
                    }
                }
                MobileForm.FormTextFieldDelegate {
                    label: i18n("Ringtone")
                    text: Config.ringtone
                    enabled: true //TODO
                    onEditingFinished: {
                        // TODO
                    }
                }
                //TODO file chooser
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18n("Default Devices")
                }

                MobileForm.FormComboBoxDelegate {
                    text: i18n("Microphone")
                    description: i18n("This microphone will be used by default during calls. You can also switch the microphone during calls.")
                    model: AudioSources
                    enabled: true //TODO
                    onCurrentIndexChanged: {
                        // TODO
                    }
                }
                MobileForm.FormComboBoxDelegate {
                    text: i18n("Camera")
                    description: i18n("This camera will be used by default during calls. You can also switch the camera during calls.")
                    model: VideoSources
                    enabled: true // TODO
                    onCurrentIndexChanged: {
                        // TODO
                    }
                }
            }
        }
    }
}
