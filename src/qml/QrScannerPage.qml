// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
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

    Connections {
        target: root.QQC2.ApplicationWindow.window
        function onClosing() {
            camera.stop();
            camera.destroy();
            connections.enabled = false;
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
                RoomManager.resolveResource(result.text, "");
                scanner.previousText = result.text;
            }
        }
        videoSink: viewFinder.videoSink
    }

    Component.onCompleted: camera.start()

    CaptureSession {
        camera: Camera {
            id: camera
        }
        imageCapture: ImageCapture {
            id: imageCapture
        }
        videoOutput: viewFinder
    }

    Connections {
        id: connections
        target: RoomManager
        function onShowUserDetail(user) {
            userDetailDialog.createObject(root.QQC2.ApplicationWindow.window, {
                user: QmlUtils.getUser(user),
                connection: root.connection
            }).open();
            root.closeDialog();
        }
        function onAskJoinRoom(room) {
            joinRoomDialog.createObject(applicationWindow(), {
                room: room,
                connection: root.connection
            }).open();
            root.closeDialog();
        }
        function onCurrentRoomChanged() {
            root.closeDialog();
        }
        function onExternalUrl(url) {
            let dialog = Qt.createComponent("org.kde.neochat", "ConfirmUrlDialog.qml").createObject(applicationWindow());
            dialog.link = url;
            dialog.open();
            root.closeDialog();
        }
    }
    Component {
        id: userDetailDialog
        UserDetailDialog {}
    }
    Component {
        id: joinRoomDialog
        JoinRoomDialog {}
    }
}
