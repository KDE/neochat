// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.kirigamiaddons.labs.components as Components

import org.kde.neochat

Kirigami.Dialog {
    id: root

    property string parentId: ""

    required property NeoChatConnection connection

    signal addChild(string childId, bool setChildParent, bool canonical)
    signal newChild(string childName)

    title: i18nc("@title", "Create Room")
    implicitWidth: Kirigami.Units.gridUnit * 20
    standardButtons: Kirigami.Dialog.Cancel

    customFooterActions: [
        Kirigami.Action {
            icon.name: "list-add-symbolic"
            text: i18nc("@action:button Create new room or space", "Create")
            enabled: roomNameField.text.length > 0
            onTriggered: {
                root.connection.createRoom(roomNameField.text, "", root.parentId, false);
                root.newChild(roomNameField.text);
                root.close();
            }
        }
    ]

    Component.onCompleted: roomNameField.forceActiveFocus()

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        FormCard.FormRadioDelegate {
            id: privateTypeDelegate
            text: i18nc("@info:label", "Private")
            description: i18nc("@info:label", "This room can only be joined with an invite.")
            checked: true
        }

        FormCard.FormRadioDelegate {
            id: publicTypeDelegate
            text: i18nc("@info:label", "Public")
            description: i18nc("@info:label", "This room can be found and joined by anyone.")
        }

        FormCard.FormDelegateSeparator {}

        FormCard.FormTextFieldDelegate {
            id: roomNameField
            label: i18n("Name:")
            placeholderText: i18nc("@info:placeholder", "New Room")
        }

        FormCard.FormTextFieldDelegate {
            id: roomAddressField
            label: i18n("Address:")
            placeholderText: i18nc("@info:placeholder", "new-room")
            visible: publicTypeDelegate.checked
        }
    }
}
