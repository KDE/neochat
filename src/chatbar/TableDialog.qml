// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtCore
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Dialogs

FormCard.FormCardDialog {
    id: root

    readonly property alias rows: rowsSpinBox.value
    readonly property alias cols: colsSpinBox.value

    title: i18nc("@title:window", "Insert Table")
    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel

    FormCard.FormSpinBoxDelegate {
        id: rowsSpinBox
        label: i18nc("@label:textbox", "Number of Rows:")
    }

    FormCard.FormDelegateSeparator {}

    FormCard.FormSpinBoxDelegate {
        id: colsSpinBox
        label: i18nc("@label:textbox", "Number of Columns:")
    }
}
