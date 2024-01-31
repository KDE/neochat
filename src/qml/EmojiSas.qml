// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQml

import com.github.quotient_im.libquotient

import org.kde.kirigami as Kirigami
import org.kde.neochat

Column {
    id: root

    required property var model

    signal accept
    signal reject

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
            onClicked: root.reject()
        }
    }
}
