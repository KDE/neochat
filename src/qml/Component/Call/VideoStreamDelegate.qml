// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.freedesktop.gstreamer.GLVideoItem 1.0

import org.kde.neochat 1.0

Rectangle {
    id: videoStreamDelegate

    implicitWidth: height / 9 * 16
    implicitHeight: 300
    color: "black"
    radius: 10

    QQC2.Label {
        anchors.top: parent.top
        anchors.horizontalCenter: parent.horizontalCenter
        color: "white"
        text: model.object.user.id
    }

    RowLayout {
        anchors.fill: parent
        Loader {
            active: model.object.hasCamera
            Layout.maximumWidth: parent.width
            Layout.maximumHeight: parent.height
            Layout.preferredHeight: parent.height
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            onActiveChanged: if (active) model.object.initCamera(camera)
            Component.onCompleted: if (active) model.object.initCamera(camera)
            GstGLVideoItem {
                id: camera
                width: parent.width
                height: parent.height
            }
        }
        Loader {
            active: false
            Layout.maximumWidth: parent.width
            Layout.maximumHeight: parent.height
            Layout.preferredHeight: parent.height
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            GstGLVideoItem {
                id: screenCast
                width: parent.width
                height: parent.height

                Component.onCompleted: {
                    //model.object.initCamera(this)
                }
            }
        }
    }
}
