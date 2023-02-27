// SPDX-FileCopyrightText: 2020 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

LoginStep {
    id: loginMethod

    title: i18n("Login Methods")

    Layout.alignment: Qt.AlignHCenter

    QQC2.Button {
        Layout.alignment: Qt.AlignHCenter
        text: i18n("Login with password")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 12
        onClicked: processed("qrc:/Password.qml")
    }

    QQC2.Button {
        Layout.alignment: Qt.AlignHCenter
        text: i18n("Login with single sign-on")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 12
        onClicked: processed("qrc:/Sso.qml")
    }
}
