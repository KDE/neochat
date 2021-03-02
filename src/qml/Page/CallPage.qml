/* SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.14 as Kirigami

import QtGraphicalEffects 1.15

import org.kde.neochat 1.0

Kirigami.Page {
    id: page

    title: CallManager.hasInvite ? i18n("Incoming Call")
            : CallManager.isInviting ? i18n("Calling")
            : CallManager.state == CallSession.Initiating ? i18n("Configuring Call")
            : i18n("Call")

    ColumnLayout {
        id: column
        anchors.fill: parent

        RowLayout {
            id: streams
            Layout.fillWidth: true
            Layout.fillHeight: true
            Repeater {
                id: videos
                model: CallManager.callParticipants
                delegate: VideoStreamDelegate {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignHCenter
                }
            }
        }


        Kirigami.Avatar {
            visible: videos.count === 0
            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
            Layout.alignment: Qt.AlignHCenter

            name: CallManager.room.displayName
            source: "image://mxc/" + CallManager.room.avatarMediaId
        }

        //QQC2.Label {
            //text: CallManager.remoteUser.displayName

            //horizontalAlignment: Text.AlignHCenter
            //Layout.fillWidth: true
        //}

        //QQC2.Label {
            //text: CallManager.room.displayName

            //horizontalAlignment: Text.AlignHCenter
            //Layout.fillWidth: true
        //}

        RowLayout {
            Layout.alignment: Qt.AlignHCenter

            id: buttonRow
            spacing: Kirigami.Units.gridUnit

            CallPageButton {
                text: i18n("Accept")
                icon.name: "call-start"
                shimmering: true
                temprament: CallPageButton.Constructive
                visible: CallManager.globalState === CallManager.INCOMING

                onClicked: {
                    visible = false; //TODO declarify
                    CallManager.acceptCall()
                }
            }
            CallPageButton {
                text: checked ? i18n("Enable Camera") : i18n("Disable Camera")
                icon.name: checked ? "camera-off" : "camera-on"
                checkable: true
                onToggled: CallManager.toggleCamera()
            }
            CallPageButton {
                text: checked ? i18n("Unmute Speaker") : i18n("Mute Speaker")
                icon.name: checked ? "audio-volume-muted" : "audio-speakers-symbolic"
                checkable: true
            }
            CallPageButton {
                text: checked ? i18n("Unmute Microphone") : i18n("Mute Microphone")
                icon.name: checked ? "microphone-sensitivity-muted" : "microphone-sensitivity-high"
                checkable: true
                checked: CallManager.muted

                onToggled: CallManager.muted = !CallManager.muted
            }
            CallPageButton {
                text: i18n("Configure Devices")
                icon.name: "settings-configure"
                onClicked: callConfigurationSheet.open()
            }
            CallPageButton {
                id: denyButton
                visible: CallManager.globalState === CallManager.INCOMING
                text: i18n("Deny")
                icon.name: "call-stop"
                shimmering: true
                temprament: CallPageButton.Destructive

                onClicked: CallManager.hangupCall()
            }
            CallPageButton {
                visible: !denyButton.visible
                text: CallManager.isInviting ? i18n("Cancel") : i18n("Hang Up")
                icon.name: "call-stop"
                shimmering: CallManager.isInviting
                temprament: CallPageButton.Destructive

                onClicked: CallManager.hangupCall()
            }
        }
    }

    Connections {
        target: CallManager
        function onCallEnded() {
            page.closeDialog()
        }
    }
}
