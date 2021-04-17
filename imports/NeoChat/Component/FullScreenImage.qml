// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15

import org.kde.kirigami 2.15 as Kirigami

ApplicationWindow {
    id: root

    property string filename
    property url localPath

    flags: Qt.FramelessWindowHint | Qt.WA_TranslucentBackground
    visibility: Qt.WindowFullScreen

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
	visible: image.status !== Image.Ready
	anchors.centerIn: parent
	running: visible
    }

    AnimatedImage {
	id: image
        anchors.centerIn: parent

        width: Math.min(sourceSize.width, root.width)
        height: Math.min(sourceSize.height, root.height)

        cache: false
        fillMode: Image.PreserveAspectFit

        source: localPath
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
