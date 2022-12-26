// SPDX-FileCopyrightText: 2019-2020 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    id: root

    property var room
    property string needUpgradeRoom: i18n("You need to upgrade this room to a newer version to enable this setting.")

    title: i18n("Security")

    leftPadding: 0
    rightPadding: 0

    ColumnLayout {
        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18nc("@option:check", "Access")
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("@option:check", "Private (invite only)")
                    description: i18n("Only invited people can join.")
                    checked: room.joinRule === "invite"
                    enabled: room.canSendState("m.room.join_rules")
                    onCheckedChanged: if (checked) {
                        room.joinRule = "invite";
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("@option:check", "Space members")
                    description: i18n("Anyone in a space can find and join.") +
                                 (!["8", "9", "10"].includes(room.version) ? `\n${needUpgradeRoom}` : "")
                    checked: room.joinRule === "restricted"
                    enabled: room.canSendState("m.room.join_rules") && ["8", "9", "10"].includes(room.version) && false
                    onCheckedChanged: if (checked) {
                        room.joinRule = "restricted";
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("@option:check", "Knock")
                    description: i18n("People not in the room need to request an invite to join the room.") +
                                 (!["7", "8", "9", "10"].includes(room.version) ? `\n${needUpgradeRoom}` : "")
                    checked: room.joinRule === "knock"
                    // https://spec.matrix.org/v1.4/rooms/#feature-matrix
                    enabled: room.canSendState("m.room.join_rules") && ["7", "8", "9", "10"].includes(room.version)
                    onCheckedChanged: if (checked) {
                        room.joinRule = "knock";
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("@option:check", "Public")
                    description: i18nc("@option:check", "Anyone can find and join.")
                    checked: room.joinRule === "public"
                    enabled: room.canSendState("m.room.join_rules")
                    onCheckedChanged: if (checked) {
                        room.joinRule = "public";
                    }
                }
            }
        }

        MobileForm.FormCard {
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.fillWidth: true
            contentItem: ColumnLayout {
                spacing: 0
                MobileForm.FormCardHeader {
                    title: i18nc("@option:check", "Message history visibility")
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("@option:check", "Anyone")
                    description: i18nc("@option:check", "Anyone, regardless of whether they have joined, can view history.")
                    checked: room.historyVisibility === "world_readable"
                    enabled: room.canSendState("m.room.history_visibility")
                    onCheckedChanged: if (checked) {
                        room.historyVisibility = "world_readable"
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("@option:check", "Members only")
                    description: i18nc("@option:check", "All members can view the entire message history, even before they joined.")
                    checked: room.historyVisibility === "shared"
                    enabled: room.canSendState("m.room.history_visibility")
                    onCheckedChanged: if (checked) {
                        room.historyVisibility = "shared"
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("@option:check", "Members only (since invite)")
                    description: i18nc("@option:check", "New members can view the message history from the point they were invited to the room.")
                    checked: room.historyVisibility === "invited"
                    enabled: room.canSendState("m.room.history_visibility")
                    onCheckedChanged: if (checked) {
                        room.historyVisibility = "invited"
                    }
                }
                MobileForm.FormRadioDelegate {
                    text: i18nc("@option:check", "Members only (since joining)")
                    description: i18nc("@option:check", "New members can view the message history from the point they joined the room.")
                    checked: room.historyVisibility === "joined"
                    enabled: room.canSendState("m.room.history_visibility")
                    onCheckedChanged: if (checked) {
                        room.historyVisibility = "joined"
                    }
                }
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

