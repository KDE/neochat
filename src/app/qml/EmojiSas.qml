// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.neochat

ColumnLayout {
    id: root

    required property var model

    signal accept
    signal reject

    spacing: Kirigami.Units.largeSpacing

    Item {
        Layout.fillHeight: true
    }
    QQC2.Label {
        Layout.fillWidth: true
        text: i18n("Confirm the emoji below are displayed on both devices, in the same order.")
        horizontalAlignment: Text.AlignHCenter
        wrapMode: Text.Wrap
    }
    EmojiRow {
        Layout.maximumWidth: implicitWidth
        Layout.alignment: Qt.AlignHCenter
        model: root.model.slice(0, 4)
    }
    EmojiRow {
        Layout.maximumWidth: implicitWidth
        Layout.alignment: Qt.AlignHCenter
        model: root.model.slice(4, 7)
    }
    RowLayout {
        Layout.fillWidth: true
        Layout.alignment: Qt.AlignHCenter
        QQC2.Button {
            text: i18n("They match")
            icon.name: "dialog-ok"
            onClicked: root.accept()
        }
        QQC2.Button {
            text: i18n("They don't match")
            icon.name: "dialog-cancel"
            onClicked: root.reject()
        }
    }
    Item {
        Layout.fillHeight: true
    }
}
