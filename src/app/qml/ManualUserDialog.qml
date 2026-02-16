// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

Kirigami.Dialog {
    id: root

    /**
     * @brief The connection for the current user.
     */
    required property NeoChatConnection connection

    /**
     * @brief Thrown when a user is selected.
     */
    signal userSelected(string userId)

    title: i18nc("@title", "User ID")
    showCloseButton: false

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0
    standardButtons: QQC2.Dialog.Cancel

    onAccepted: root.userSelected(userIdText.text)

    contentItem: ColumnLayout {
        spacing: 0
        FormCard.FormTextFieldDelegate {
            id: userIdText
            property bool isValidText: text.match(/@(.+):(.+)/g)
            property bool correctStart: text.startsWith("@")

            label: i18n("User ID:")
            statusMessage: {
                if (text.length > 0 && !correctStart) {
                    return i18n("User IDs Must start with @");
                }
                if (timer.running) {
                    return "";
                }
                if (text.length > 0 && !isValidText) {
                    return i18n("The input is not a valid user ID");
                }
                return correctStart ? "" : i18n("User IDs Must start with @");
            }
            status: text.length > 0 ? Kirigami.MessageType.Error : Kirigami.MessageType.Information

            onTextEdited: timer.restart()

            Timer {
                id: timer
                interval: 1000
            }
        }
    }

    onVisibleChanged: {
        userIdText.forceActiveFocus();
        timer.restart();
    }

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: i18nc("@action:button Perform an action with this user ID", "Ok")
            icon.name: "checkmark"
            enabled: userIdText.isValidText

            onClicked: root.accept()

            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
        }
    }
}
