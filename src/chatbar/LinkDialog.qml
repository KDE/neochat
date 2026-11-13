// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.ki18n

FormCard.FormCardDialog {
    id: root

    property alias linkText: linkTextField.text
    property alias linkUrl: linkUrlField.text

    title: KI18n.i18nc("@title:window", "Insert Link")
    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel

    FormCard.FormTextFieldDelegate {
        id: linkTextField

        label: KI18n.i18nc("@label:textbox", "Link Text:")
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormTextFieldDelegate {
        id: linkUrlField

        label: KI18n.i18nc("@label:textbox", "Link URL:")
    }
}
