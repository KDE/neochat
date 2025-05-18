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

    property string parentId

    required property NeoChatConnection connection

    signal newChild(string childName)

    title: i18nc("@title", "Create a Space")
    implicitWidth: Kirigami.Units.gridUnit * 20
    standardButtons: Kirigami.Dialog.Cancel

    Component.onCompleted: roomNameField.forceActiveFocus()

    customFooterActions: [
        Kirigami.Action {
            icon.name: "list-add-symbolic"
            text: i18nc("@action:button Create new space", "Create")
            enabled: roomNameField.text.length > 0
            onTriggered: {
                root.connection.createSpace(roomNameField.text, "", root.parentId, newOfficialCheck.checked);
                root.newChild(roomNameField.text);
                root.close();
            }
        }
    ]

    ColumnLayout {
        spacing: Kirigami.Units.largeSpacing

        FormCard.FormTextFieldDelegate {
            id: roomNameField
            label: i18nc("@info:label Name of the space", "Name:")
            placeholderText: i18nc("@info:placeholder", "New Space")
        }

        FormCard.FormDelegateSeparator {
            above: roomNameField
            below: newOfficialCheck
            visible: newOfficialCheck.visible
        }

        FormCard.FormCheckDelegate {
            id: newOfficialCheck
            visible: root.parentId.length > 0
            text: i18nc("@option:check As in make the space from which this dialog was created an official parent.", "Make this parent official")
            checked: true
        }
    }
}
