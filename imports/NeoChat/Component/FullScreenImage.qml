/**
 * SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-LicenseIdentifier: GPL-3.0-only
 */

import QtQuick 2.12
import QtQuick.Controls 2.12

import org.kde.kirigami 2.12 as Kirigami

ApplicationWindow {
    id: root

    property string filename
    property url localPath

    flags: Qt.FramelessWindowHint | Qt.WA_TranslucentBackground
    visibility: Qt.WindowFullScreen

    title: i18n("Image View - %1", filename)

    color: "#BB000000"

    Shortcut {
        sequence: "Escape"
        onActivated: root.destroy()
    }

    AnimatedImage {
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
