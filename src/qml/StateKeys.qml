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
                onClicked: applicationWindow().pageStack.pushDialogLayer('qrc:/org/kde/neochat/qml/MessageSourceSheet.qml', {
                    sourceText: stateKeysModel.stateEventJson(stateKeysModel.index(model.index, 0))
                }, {
                    title: i18nc("@title:window", "Event Source"),
                    width: Kirigami.Units.gridUnit * 25
                })
            }
        }
    }
}
