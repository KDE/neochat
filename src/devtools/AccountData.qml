// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQuick.Window

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

ColumnLayout {
    id: root

    required property NeoChatConnection connection

    FormCard.FormHeader {
        title: i18nc("@title:group", "Account Data")
    }
    FormCard.FormCard {
        Repeater {
            model: root.connection.accountDataEventTypes
            delegate: FormCard.FormButtonDelegate {
                text: modelData
                onClicked: root.Window.window.pageStack.pushDialogLayer(Qt.createComponent('org.kde.neochat', 'MessageSourceSheet'), {
                    sourceText: root.connection.accountDataJsonString(modelData)
                }, {
                    title: i18nc("@title:window", "Event Source"),
                    width: Kirigami.Units.gridUnit * 25
                })
            }
        }
    }
    FormCard.FormCard {
        FormCard.FormSwitchDelegate {
            id: showAccessTokenCheckbox
            text: i18nc("@info", "Show Access Token")
            description: i18n("This should not be shared with anyone, even other users. This token gives full access to your account.")
        }
        FormCard.FormTextDelegate {
            text: i18nc("@info", "Access Token")
            description: root.connection.accessToken
            visible: showAccessTokenCheckbox.checked

            contentItem.children: QQC2.Button {
                text: i18nc("@action:button", "Copy access token to clipboard")
                icon.name: "edit-copy"
                display: QQC2.AbstractButton.IconOnly

                onClicked: Clipboard.saveText(root.connection.accessToken)

                QQC2.ToolTip.text: text
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                QQC2.ToolTip.visible: hovered
            }
        }
    }
}
