// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.kirigami 2.15 as Kirigami

ApplicationWindow {
    id: root

    property alias source: image.source
    property string filename
    property string blurhash: ""
    property int imageWidth: -1
    property int imageHeight: -1

    flags: Qt.FramelessWindowHint | Qt.WA_TranslucentBackground

    title: i18n("Image View - %1", filename)

    Shortcut {
        sequence: "Escape"
        onActivated: root.destroy()
    }

    color: Kirigami.Theme.backgroundColor

    background: AbstractButton {
        onClicked: root.destroy()
    }

    BusyIndicator {
        visible: image.status !== Image.Ready && root.blurhash === ""
        anchors.centerIn: parent
        running: visible
    }

    AnimatedImage {
	    id: image
        anchors.centerIn: parent

        width: Math.min(root.imageWidth !== -1 ? root.imageWidth : sourceSize.width, root.width)
        height: Math.min(root.imageHeight !== -1 ? root.imageWidth : sourceSize.height, root.height)

        fillMode: Image.PreserveAspectFit

        Image {
            anchors.centerIn: parent
            width: image.width
            height: image.height
            source: root.blurhash !== "" ? ("image://blurhash/" + root.blurhash) : ""
            visible: root.blurhash !== "" && parent.status !== Image.Ready
        }
    }

    Button {
        anchors.top: parent.top
        anchors.right: parent.right

        text: i18n("Close")
        icon.name: "dialog-close"
        display: AbstractButton.IconOnly

        width: Kirigami.Units.gridUnit * 2
        height: Kirigami.Units.gridUnit * 2

        onClicked: root.destroy()
    }
}
