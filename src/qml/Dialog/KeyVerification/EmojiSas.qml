// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQml 2.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.neochat 1.0

Column {
    id: root

    required property var model

    signal accept()
    signal reject()

    visible: dialog.session.state === KeyVerificationSession.WAITINGFORVERIFICATION
    anchors.centerIn: parent
    spacing: Kirigami.Units.largeSpacing
    QQC2.Label {
        text: i18n("Confirm the emoji below are displayed on both devices, in the same order.")
    }
    EmojiRow {
        anchors.horizontalCenter: parent.horizontalCenter
        height: Kirigami.Units.gridUnit * 4
        model: root.model.slice(0, 4)
    }
    EmojiRow {
        anchors.horizontalCenter: parent.horizontalCenter
        height: Kirigami.Units.gridUnit * 4
        model: root.model.slice(4, 7)
    }
    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        QQC2.Button {
            anchors.bottom: parent.bottom
            text: i18n("They match")
            icon.name: "dialog-ok"
            onClicked: root.accept()
        }
        QQC2.Button {
            anchors.bottom: parent.bottom
            text: i18n("They don't match")
            icon.name: "dialog-cancel"
            onClicked:  root.reject()
        }
    }
}
