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

    readonly property alias imagePath: imageField.path

    title: i18nc("@title:window", "Insert Image")
    standardButtons: QQC2.Dialog.Ok | QQC2.Dialog.Cancel

    FileDialog {
        id: fileDialog

        title: i18nc("@title:window", "Select an image")
        currentFolder: StandardPaths.writableLocation(StandardPaths.PicturesLocation)
        fileMode: FileDialog.OpenFile
        nameFilters: [i18n("Image files (*.jpg *.jpeg *.png *.svg *.webp)"), i18n("All files (*)")]
        onAccepted: imageField.path = selectedFile
    }

    FormCard.FormButtonDelegate {
        id: imageField

        property url path

        text: i18nc("@label:textbox", "Image Location:")
        description: path.toString().length > 0 ? path.toString().split('/').slice(-1)[0] : ''

        onClicked: fileDialog.open()
    }

    Item {
        visible: imageField.path.toString().length > 0

        Layout.fillWidth: true
        Layout.preferredHeight: 200
        Layout.topMargin: Kirigami.Units.largeSpacing

        Image {
            anchors.fill: parent
            source: imageField.path
            fillMode: Image.PreserveAspectFit
            horizontalAlignment: Image.AlignHCenter
        }
    }
}
