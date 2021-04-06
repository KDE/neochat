// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component.Login 1.0

LoginStep {
    id: loginMethod

    title: i18n("Login Methods")

    Layout.alignment: Qt.AlignHCenter

    Controls.Button {
        Layout.alignment: Qt.AlignHCenter
        text: i18n("Login with password")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 12
        onClicked: processed("qrc:/imports/NeoChat/Component/Login/Password.qml")
    }

    Controls.Button {
        Layout.alignment: Qt.AlignHCenter
        text: i18n("Login with single sign-on")
        Layout.preferredWidth: Kirigami.Units.gridUnit * 12
        onClicked: processed("qrc:/imports/NeoChat/Component/Login/Sso.qml")
    }
}
