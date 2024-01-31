// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

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
            Registration.registerAccount();
        }
    }
    previousAction: Kirigami.Action {
        onTriggered: root.processed("qrc:/org/kde/neochat/qml/Username.qml")
    }
}
