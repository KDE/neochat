// SPDX-FileCopyrightText: 2026 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-3.0-only

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

Kirigami.Dialog {
    id: root

    required property NeoChatConnection connection

    readonly property SupportController supportController: SupportController {
        connection: root.connection
    }
    readonly property bool hasSupportResources: supportController.supportPage.length > 0 && supportController.contacts.length > 0

    title: i18nc("@title Support information", "Support")
    width: Math.min(Kirigami.Units.gridUnit * 30, QQC2.ApplicationWindow.window.width)

    ColumnLayout {
        spacing: 0

        FormCard.FormTextDelegate {
            id: explanationTextDelegate

            text: root.hasSupportResources ?
                i18nc("@info:label %1 is the domain of the server", "Official support resources provided by %1:", root.connection.domain)
                : i18nc("@info:label %1 is the domain of the server", "%1 has no support resources.", root.connection.domain)
        }

        FormCard.FormDelegateSeparator {
            above: explanationTextDelegate
            below: openSupportPageDelegate
            visible: openSupportPageDelegate.visible
        }

        FormCard.FormLinkDelegate {
            id: openSupportPageDelegate

            icon.name: "help-contents-symbolic"
            text: i18nc("@action:button Open support webpage", "Open Support")
            url: root.supportController.supportPage
            visible: root.supportController.supportPage.length > 0
        }

        FormCard.FormDelegateSeparator {
            above: openSupportPageDelegate
            visible: root.supportController.contacts.length > 0
        }

        Repeater {
            model: root.supportController.contacts

            delegate: FormCard.AbstractFormDelegate {
                id: contactDelegate

                required property string role
                required property string matrixId
                required property string emailAddress

                background: null

                Layout.fillWidth: true

                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing

                    Kirigami.Icon {
                        source: "user"
                    }

                    QQC2.Label {
                        text: {
                            // Translate known keys
                            if (contactDelegate.role === "m.role.admin") {
                                return i18nc("@info:label Adminstrator contact", "Admin")
                            } else if (contactDelegate.role === "m.role.security") {
                                return i18nc("@info:label Security contact", "Security")
                            }
                            return contactDelegate.role;
                        }
                        elide: Text.ElideRight

                        Layout.fillWidth: true
                    }

                    QQC2.ToolButton {
                        visible: contactDelegate.matrixId.length > 0
                        icon.name: "document-send-symbolic"

                        onClicked: {
                            root.close();
                            root.connection.requestDirectChat(contactDelegate.matrixId);
                        }

                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip %1 is a Matrix ID", "Contact via Matrix (%1)", contactDelegate.matrixId)
                    }

                    QQC2.ToolButton {
                        visible: contactDelegate.emailAddress.length > 0
                        icon.name: "mail-sent-symbolic"

                        onClicked: Qt.openUrlExternally("mailto:%1".arg(contactDelegate.emailAddress))

                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.text: i18nc("@info:tooltip %1 is an e-mail address", "Contact via e-mail (%1)", contactDelegate.emailAddress)
                    }
                }
            }
        }
    }
}
