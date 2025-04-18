// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtMultimedia

import org.kde.kirigami as Kirigami
import org.kde.prison.scanner as Prison

import org.kde.neochat

Kirigami.Page {
    id: root

    title: i18nc("@title", "Scan a QR Code")

    required property NeoChatConnection connection
    padding: 0

    Component.onCompleted: camera.start()

    Connections {
        target: root.QQC2.ApplicationWindow.window
        function onClosing() {
            root.destroy();
        }
    }

    VideoOutput {
        id: viewFinder
        anchors.centerIn: parent
    }

    Prison.VideoScanner {
        id: scanner
        property string previousText: ""
        formats: Prison.Format.QRCode | Prison.Format.Aztec
        onResultChanged: {
            if (result.text.length > 0 && result.text != scanner.previousText) {
                RoomManager.resolveResource(result.text, "qr");
                scanner.previousText = result.text;
            }
            root.closeDialog();
        }
        videoSink: viewFinder.videoSink
    }

    CaptureSession {
        camera: Camera {
            id: camera
        }
        imageCapture: ImageCapture {
            id: imageCapture
        }
        videoOutput: viewFinder
    }
}
