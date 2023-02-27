// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15
import QtQml 2.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.neochat 1.0

Kirigami.Page {
    id: dialog
    title: i18n("Session Verification")

    required property var session

    Item {
        anchors.fill: parent
        VerificationCanceled {
            visible: dialog.session.state === KeyVerificationSession.CANCELED
            anchors.centerIn: parent
            reason: dialog.session.error
        }
        EmojiSas {
            anchors.centerIn: parent
            visible: dialog.session.state === KeyVerificationSession.WAITINGFORVERIFICATION
            model: dialog.session.sasEmojis
            onReject: dialog.session.cancelVerification(KeyVerificationSession.MISMATCHED_SAS)
            onAccept: dialog.session.sendMac()
        }
        Message {
            visible: dialog.session.state === KeyVerificationSession.WAITINGFORREADY
            anchors.centerIn: parent
            icon: "security-medium-symbolic"
            text: i18n("Waiting for device to accept verification.")
        }
        Message {
            visible: dialog.session.state === KeyVerificationSession.INCOMING
            anchors.centerIn: parent
            icon: "security-medium-symbolic"
            text: i18n("Incoming key verification request from device **%1**", dialog.session.remoteDeviceId)
        }
        Message {
            visible: dialog.session.state === KeyVerificationSession.WAITINGFORMAC
            anchors.centerIn: parent
            icon: "security-medium-symbolic"
            text: i18n("Waiting for other party to verify.")
        }
        Kirigami.BasicListItem {
            id: emojiVerification
            text: "Emoji Verification"
            visible: dialog.session.state === KeyVerificationSession.READY
            subtitle: i18n("Compare a set of emoji on both devices")
            onClicked: {
                dialog.session.sendStartSas()
            }
        }
        Message {
            visible: dialog.session.state === KeyVerificationSession.DONE
            anchors.centerIn: parent
            text: i18n("Successfully verified device **%1**", dialog.session.remoteDeviceId)
            icon: "security-high"
        }
    }

    footer: QQC2.ToolBar {
        visible: dialog.session.state === KeyVerificationSession.INCOMING
        QQC2.DialogButtonBox {
            anchors.fill: parent
            Item { Layout.fillWidth: true }
            QQC2.Button {
                text: i18n("Accept")
                icon.name: "dialog-ok"
                onClicked: dialog.session.sendReady()
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            }
            QQC2.Button {
                text: i18n("Decline")
                icon.name: "dialog-cancel"
                onClicked: dialog.session.cancelVerification("m.user", "Declined")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.CancelRole
            }
        }
    }
}
