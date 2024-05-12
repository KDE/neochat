// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.AbstractFormDelegate {
    id: root

    required property NeoChatConnection connection

    property bool editServerUrl: false

    text: identityServerHelper.currentServer

    onClicked: editIdServerButton.toggle()

    contentItem: RowLayout {
        spacing: Kirigami.Units.largeSpacing

        QQC2.Label {
            Layout.fillWidth: true
            visible: !root.editServerUrl
            text: root.text
            elide: Text.ElideRight
        }
        ColumnLayout {
            Kirigami.ActionTextField {
                id: editUrlField
                Layout.fillWidth: true
                visible: root.editServerUrl

                Accessible.description: i18n("New identity server url")

                rightActions: [
                    Kirigami.Action {
                        text: i18nc("@action:button", "Cancel editing identity server url")
                        icon.name: "edit-delete-remove"
                        onTriggered: editIdServerButton.toggle()
                    },
                    Kirigami.Action {
                        enabled: identityServerHelper.status == IdentityServerHelper.Valid
                        text: i18nc("@action:button", "Confirm new identity server url")
                        icon.name: "checkmark"
                        visible: editUrlField.text !== root.text
                        onTriggered: {
                            identityServerHelper.setIdentityServer();
                            editUrlField.text = "";
                            editIdServerButton.toggle();
                        }
                    }
                ]

                onAccepted: {
                    identityServerHelper.setIdentityServer()
                    editUrlField.text = "";
                    editIdServerButton.toggle();
                }
            }
            Kirigami.InlineMessage {
                id: editUrlStatus
                visible: root.editServerUrl && text.length > 0 && !warningTimer.running
                Layout.topMargin: visible ? Kirigami.Units.smallSpacing : 0
                Layout.fillWidth: true
                text: switch(identityServerHelper.status) {
                case IdentityServerHelper.Invalid:
                    return i18n("The entered url is not a valid identity server");
                case IdentityServerHelper.Match:
                    return i18n("The entered url is already configured as your identity server");
                default:
                    return "";
                }

                type: switch(identityServerHelper.status) {
                case IdentityServerHelper.Invalid:
                    return Kirigami.MessageType.Error;
                case IdentityServerHelper.Match:
                    return Kirigami.MessageType.Warning;
                default:
                    return Kirigami.MessageType.Information;
                }

                Timer {
                    id: warningTimer
                    interval: 500
                }
            }
        }
        QQC2.ToolButton {
            id: editIdServerButton
            display: QQC2.AbstractButton.IconOnly
            text: i18nc("@action:button", "Edit identity server url")
            icon.name: "document-edit"
            checkable: true
            onCheckedChanged: {
                root.editServerUrl = !root.editServerUrl;
                if (checked) {
                    editUrlField.forceActiveFocus();
                } else {
                    editUrlField.text = "";
                }
            }
            QQC2.ToolTip {
                text: editIdServerButton.text
                delay: Kirigami.Units.toolTipDelay
                visible: editIdServerButton.hovered
            }
        }
        QQC2.ToolButton {
            id: removeIdServerButton
            visible: identityServerHelper.hasCurrentServer
            display: QQC2.AbstractButton.IconOnly
            text: i18nc("@action:button", "Remove identity server")
            icon.name: "edit-delete-remove"
            onClicked: {
                identityServerHelper.clearIdentityServer();
                editUrlField.text = "";
                if (editIdServerButton.checked) {
                    editIdServerButton.toggle();
                }
            }
            QQC2.ToolTip {
                text: removeIdServerButton.text
                delay: Kirigami.Units.toolTipDelay
                visible: removeIdServerButton.hovered
            }
        }
    }

    IdentityServerHelper {
        id: identityServerHelper
        connection: root.connection
        url: editUrlField.text
        onUrlChanged: warningTimer.restart()
    }
}
