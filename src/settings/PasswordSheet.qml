// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

QQC2.Dialog {
    id: root

    signal submitPassword(string password)

    title: i18nc("@title:dialog", "Enter password")

    preferredWidth: Kirigami.Units.gridUnit * 24

    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel
    onAccepted: {
        root.submitPassword(passwordField.text);
        passwordField.text = "";
        root.close();
    }

    ColumnLayout {
        FormCard.FormTextFieldDelegate {
            id: passwordField
            label: i18nc("@label:textbox", "Password:")
            echoMode: TextInput.Password
        }
    }
}
