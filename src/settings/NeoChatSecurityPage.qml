// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title", "Security")

    FormCard.FormHeader {
        title: i18nc("@title", "Keys")
    }
    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: connection.deviceKey
            description: i18n("Device key")
        }
        FormCard.FormTextDelegate {
            text: connection.encryptionKey
            description: i18n("Encryption key")
        }
        FormCard.FormTextDelegate {
            text: connection.deviceId
            description: i18n("Device id")
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title", "Encryption Keys")
    }
    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Import Encryption Keys")
            icon.name: "document-import"
            onClicked: pageStack.pushDialogLayer(Qt.createComponent("org.kde.neochat.settings", "ImportKeysDialog"), {
                connection: root.connection
            }, {
                title: i18nc("@title", "Import Keys")
            })
        }
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Export Encryption Keys")
            icon.name: "document-export"
            onClicked: pageStack.pushDialogLayer(Qt.createComponent("org.kde.neochat.settings", "ExportKeysDialog"), {
                connection: root.connection
            }, {
                title: i18nc("@title", "Export Keys")
            })
        }
    }

    FormCard.FormHeader {
        title: i18nc("@title:group", "Ignored Users")
    }
    FormCard.FormCard {
        FormCard.FormButtonDelegate {
            text: i18nc("@action:button", "Manage ignored users")
            onClicked: pageStack.pushDialogLayer(ignoredUsersDialogComponent, {}, {
                title: i18nc("@title:window", "Ignored Users")
            });
        }
    }

    Component {
        id: ignoredUsersDialogComponent
        IgnoredUsersDialog {
            connection: root.connection
        }
    }
}
