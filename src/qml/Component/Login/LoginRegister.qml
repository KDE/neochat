// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

LoginStep {
    id: loginRegister

    Layout.alignment: Qt.AlignHCenter

    QQC2.Button {
        Layout.alignment: Qt.AlignHCenter
        text: i18n("Login")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 12
        onClicked: processed("qrc:/Login.qml")
    }

    QQC2.Button {
        Layout.alignment: Qt.AlignHCenter
        text: i18n("Register")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 12
        onClicked: processed("qrc:/Homeserver.qml")
    }
}
