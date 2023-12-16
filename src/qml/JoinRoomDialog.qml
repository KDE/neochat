// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.components as KirigamiComponents
import org.kde.kirigamiaddons.formcard as FormCard
import org.kde.prison

import org.kde.neochat

Kirigami.Dialog {
    id: root

    property string room
    property NeoChatConnection connection

    parent: applicationWindow().overlay

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    standardButtons: Kirigami.Dialog.NoButton

    width: Math.min(applicationWindow().width, Kirigami.Units.gridUnit * 24)
    title: i18nc("@title", "Join Room")

    contentItem: ColumnLayout {
        spacing: 0
        RowLayout {
            Layout.fillWidth: true
            Layout.leftMargin: Kirigami.Units.largeSpacing
            Layout.rightMargin: Kirigami.Units.largeSpacing
            Layout.topMargin: Kirigami.Units.largeSpacing
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            spacing: Kirigami.Units.largeSpacing

            KirigamiComponents.Avatar {
                id: avatar
                Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                Layout.preferredHeight: Kirigami.Units.iconSizes.huge

                name: root.room.slice(1, -1)
                initialsMode: KirigamiComponents.Avatar.UseInitials
            }

            ColumnLayout {
                Layout.fillWidth: true

                Kirigami.Heading {
                    level: 1
                    Layout.fillWidth: true
                    font.bold: true

                    elide: Text.ElideRight
                    wrapMode: Text.NoWrap
                    text: root.room
                    textFormat: Text.PlainText
                }
            }
            QQC2.AbstractButton {
                Layout.minimumHeight: avatar.height * 0.75
                Layout.maximumHeight: avatar.height * 1.5
                contentItem: Barcode {
                    id: barcode
                    barcodeType: Barcode.QRCode
                    content: "https://matrix.to/#/" + root.room
                }

                onClicked: {
                    let qr = qrMaximizeComponent.createObject(parent, {
                        text: barcode.content,
                        title: root.room
                    });
                    root.close();
                    qr.open();
                }

                QQC2.ToolTip.visible: hovered
                QQC2.ToolTip.text: barcode.content
                QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay
            }
        }

        Kirigami.Separator {
            Layout.fillWidth: true
        }

        FormCard.FormButtonDelegate {
            action: Kirigami.Action {
                text: i18n("Join room")
                icon.name: "irc-join-channel"
                onTriggered: {
                    RoomManager.resolveResource(root.room, "join");
                    root.close();
                }
            }
        }
    }
    Component {
        id: qrMaximizeComponent
        QrCodeMaximizeComponent {}
    }
}
