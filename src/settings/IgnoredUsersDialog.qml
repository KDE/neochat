// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormCardPage {
    id: root

    required property NeoChatConnection connection

    title: i18nc("@title", "Ignored Users")

    width: Kirigami.Units.gridUnit * 16
    height: Kirigami.Units.gridUnit * 32

    FormCard.FormHeader {
        title: i18nc("@title:group", "Ignored Users")
    }

    FormCard.FormCard {
        FormCard.FormTextDelegate {
            text: i18nc("Placeholder message when no user is ignored", "You are not ignoring any users")
            visible: repeater.count === 0
        }
        Repeater {
            id: repeater
            model: root.connection.ignoredUsers()
            delegate: FormCard.AbstractFormDelegate {
                topPadding: Kirigami.Units.smallSpacing
                bottomPadding: Kirigami.Units.smallSpacing

                background: null
                contentItem: RowLayout {
                    spacing: 0

                    QQC2.Label {
                        Layout.fillWidth: true
                        text: modelData
                        elide: Text.ElideRight
                        Accessible.ignored: true // base class sets this text on root already
                    }

                    QQC2.ToolButton {
                        text: i18nc("@action:button", "Unignore this user")
                        icon.name: "list-remove-symbolic"
                        onClicked: root.connection.removeFromIgnoredUsers(root.connection.user(modelData))
                        display: QQC2.Button.IconOnly
                        QQC2.ToolTip.text: text
                        QQC2.ToolTip.visible: hovered
                        QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 2
                    }
                }
            }
        }
    }

    Connections {
        target: root.connection
        function onIgnoredUsersListChanged() {
            repeater.model = root.connection.ignoredUsers();
        }
    }
}
