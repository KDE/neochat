// SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.Dialog {
    id: root

    property NeoChatRoom room

    ColumnLayout {
        Kirigami.Heading {
            text: i18n("Activate Encryption")
        }
        QQC2.Label {
            text: i18n("It will not be possible to deactivate the encryption after it is enabled.")
        }
    }

    x: Math.round((parent.width - width) / 2)
    y: Math.round((parent.height - height) / 2)
    modal: true

    footer: QQC2.DialogButtonBox {
        QQC2.Button {
            text: i18n("Cancel")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
            onClicked: root.close()
        }

        QQC2.Button {
            text: i18n("Activate Encryption")
            QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            onClicked: {
                root.room.activateEncryption()
                root.close();
            }
        }
    }
}
