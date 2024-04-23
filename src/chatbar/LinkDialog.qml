// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

FormCard.FormCardDialog {
    id: root

    property alias linkText: linkTextField.text
    property alias linkUrl: linkUrlField.text

    title: i18nc("@title:window", "Insert Link")
    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel

    FormCard.FormTextFieldDelegate {
        id: linkTextField

        label: i18nc("@label:textbox", "Link Text:")
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormTextFieldDelegate {
        id: linkUrlField

        label: i18nc("@label:textbox", "Link URL:")
    }
}
