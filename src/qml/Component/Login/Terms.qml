// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Layouts 1.12

import org.kde.kirigami 2.12 as Kirigami
import org.kde.kirigamiaddons.formcard 1.0 as FormCard

import org.kde.neochat 1.0

LoginStep {
    id: root

    noControls: true

    FormCard.FormTextDelegate {
        text: i18n("Terms & Conditions")
        description: i18n("By continuing with the registration, you agree to the following terms and conditions:")
    }

    Repeater {
        model: Registration.terms
        delegate: FormCard.FormTextDelegate {
            text: "<a href=\"" + modelData.url + "\">" + modelData.title + "</a>"
            onLinkActivated: Qt.openUrlExternally(modelData.url)
        }
    }

    nextAction: Kirigami.Action {
        onTriggered: {
            Registration.registerAccount()
        }
    }
    previousAction: Kirigami.Action {
        onTriggered: root.processed("qrc:/Username.qml")
    }
}
