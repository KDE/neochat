// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kitemmodels

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatRoom room
    required property string eventType

    title: root.eventType

    FormCard.FormHeader {
        title: i18nc("The name of one instance of a state of configuration. Unless you really know what you're doing, best leave this untranslated.", "State Keys")
    }
    FormCard.FormCard {
        Repeater {
            model: StateKeysModel {
                id: stateKeysModel
                room: root.room
                eventType: root.eventType
            }

            delegate: FormCard.FormButtonDelegate {
                text: model.stateKey
                onClicked: openEventSource(model.stateKey)
            }
        }
    }

    function openEventSource(stateKey: string): void {
        pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'MessageSourceSheet'), {
            model: stateKeysModel,
            allowEdit: true,
            room: root.room,
            type: root.eventType,
            stateKey: stateKey
        }, {
            title: i18nc("@title:window", "Event Source"),
            width: Kirigami.Units.gridUnit * 25
        });
    }
}
