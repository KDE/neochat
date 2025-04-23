// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

QQC2.Dialog {
    id: root

    /**
     * @brief The connection for the current user.
     */
    required property NeoChatConnection connection

    /**
     * @brief Thrown when a user is selected.
     */
    signal userSelected

    title: i18nc("@title", "User ID")

    width: Math.min(QQC2.ApplicationWindow.window.width, Kirigami.Units.gridUnit * 24)
    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

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

    footer: QQC2.DialogButtonBox {
        standardButtons: QQC2.Dialog.Cancel

        QQC2.Button {
            enabled: userIdText.isValidText
            text: i18n("OK")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            icon.name: "dialog-ok"
            onClicked: {
                root.connection.requestDirectChat(userIdText.text);
                root.accept();
            }
        }
    }

    onVisibleChanged: {
        userIdText.forceActiveFocus();
        timer.restart();
    }
}
