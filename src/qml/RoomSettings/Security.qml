// SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    property var room

    title: i18n("Security")

    ColumnLayout {
        Kirigami.FormLayout {
            Layout.fillWidth: true

            QQC2.RadioButton {
                text: i18nc("@option:check", "Private (invite only)")
                Kirigami.FormData.label: i18nc("@option:check", "Access:")
                checked: room.joinRule === "invite"
                enabled: room.canSendState("m.room.join_rules")
                onCheckedChanged: if (checked) {
                    room.joinRule = "invite";
                }
            }
            QQC2.Label {
                text: i18n("Only invited people can join.")
                font: Kirigami.Theme.smallFont
            }
            QQC2.RadioButton {
                text: i18nc("@option:check", "Space members")
                checked: room.joinRule === "restricted"
                enabled: room.canSendState("m.room.join_rules") && ["8", "9", "10"].includes(room.version) && false
                onCheckedChanged: if (checked) {
                    room.joinRule = "restricted";
                }
            }
            QQC2.Label {
                text: i18n("Anyone in a space can find and join.")
                wrapMode: Qt.Wrap
                width: root.width
                font: Kirigami.Theme.smallFont
            }
            QQC2.Label {
                font: Kirigami.Theme.smallFont
                visible: !["8", "9", "10"].includes(room.version)
                text: i18n("You need to upgrade this room to a newer version to enable this setting.")
            }
            QQC2.RadioButton {
                text: i18nc("@option:check", "Public")
                checked: room.joinRule === "public"
                enabled: room.canSendState("m.room.join_rules")
                onCheckedChanged: if (checked) {
                    room.joinRule = "public";
                }
            }
            QQC2.Label {
                text: i18nc("@option:check", "Anyone can find and join.")
                font: Kirigami.Theme.smallFont
            }
        }
    }

    footer: QQC2.ToolBar {
        contentItem: RowLayout {
            Item {
                Layout.fillWidth: true
            }
            QQC2.Button {
                Layout.alignment: Qt.AlignRight
                enabled: false
                text: i18n("Apply")
            }
        }
    }
}

