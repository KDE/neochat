// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtMultimedia

import org.kde.kirigami as Kirigami
import org.kde.coreaddons

import org.kde.neochat

QQC2.Dialog {
    id: root

    required property NeoChatRoom room

    VoiceRecorder {
        id: voiceRecorder
        readonly property bool recording: recorder.recorderState == MediaRecorder.RecordingState
        room: root.room
    }

    width: Kirigami.Units.gridUnit * 24

    standardButtons: QQC2.DialogButtonBox.Cancel

    title: i18nc("@title:dialog", "Record Voice Message")

    contentItem: ColumnLayout {
        QQC2.RoundButton {
            icon.name: voiceRecorder.recording ? "media-playback-stop" : "media-record"
            text: voiceRecorder.recording ? i18nc("@action:button Stop audio recording", "Stop Recording") : i18nc("@action:button Start audio recording", "Start Recording")
            Layout.preferredHeight: Kirigami.Units.gridUnit * 4
            Layout.preferredWidth: Kirigami.Units.gridUnit * 4
            Layout.alignment: Qt.AlignHCenter
            display: QQC2.RoundButton.IconOnly
            enabled: voiceRecorder.isSupported

            onClicked: voiceRecorder.recording ? voiceRecorder.stopRecording() : voiceRecorder.startRecording()
        }
        RowLayout {
            Layout.alignment: Qt.AlignHCenter
            QQC2.Label {
                text: i18nc("@info Duration being the length of an audio recording", "Duration: %1", Format.formatDuration(voiceRecorder.recorder.duration))
            }
        }

        Kirigami.InlineMessage {
            Layout.fillWidth: true
            text: i18nc("@info", "Voice message recording requires a newer Qt version than is currently installed on this system.")
            visible: !voiceRecorder.isSupported
        }
    }

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: i18nc("@action:button Send the voice message", "Send")
            icon.name: "document-send"
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            onClicked: voiceRecorder.send()
            enabled: !voiceRecorder.recording && voiceRecorder.recorder.duration > 0 && voiceRecorder.isSupported
        }
    }
}
