// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.Dialog {
    id: root

    required property NeoChatConnection connection

    ColumnLayout {
        Kirigami.Heading {
            text: i18n("Sign out")
        }
        QQC2.Label {
            text: i18n("Are you sure you want to sign out?")
        }
    }

    x: parent ? Math.round((parent.width - width) / 2) : 0
    y: parent ? Math.round((parent.height - height) / 2) : 0
    modal: true

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: i18n("Cancel")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
            onClicked: root.close()
        }

        QQC2.Button {
            text: i18n("Sign out")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            onClicked: {
                root.connection.logout(true);
                root.close();
                root.accepted();
            }
        }
    }
}
