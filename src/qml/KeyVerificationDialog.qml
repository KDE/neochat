// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQml

import com.github.quotient_im.libquotient

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.neochat

Kirigami.Page {
    id: root

    title: i18n("Session Verification")

    required property var session

    Item {
        anchors.fill: parent
        VerificationCanceled {
            visible: root.session.state === KeyVerificationSession.CANCELED
            anchors.centerIn: parent
            reason: root.session.error
        }
        EmojiSas {
            anchors.centerIn: parent
            visible: root.session.state === KeyVerificationSession.WAITINGFORVERIFICATION
            model: root.session.sasEmojis
            onReject: root.session.cancelVerification(KeyVerificationSession.MISMATCHED_SAS)
            onAccept: root.session.sendMac()
        }
        Message {
            visible: root.session.state === KeyVerificationSession.WAITINGFORREADY
            anchors.centerIn: parent
            icon: "security-medium-symbolic"
            text: i18n("Waiting for device to accept verification.")
        }
        Message {
            visible: root.session.state === KeyVerificationSession.INCOMING
            anchors.centerIn: parent
            icon: "security-medium-symbolic"
            text: i18n("Incoming key verification request from device **%1**", root.session.remoteDeviceId)
        }
        Message {
            visible: root.session.state === KeyVerificationSession.WAITINGFORMAC
            anchors.centerIn: parent
            icon: "security-medium-symbolic"
            text: i18n("Waiting for other party to verify.")
        }
        Delegates.RoundedItemDelegate {
            id: emojiVerification
            text: i18n("Emoji Verification")
            visible: root.session.state === KeyVerificationSession.READY
            contentItem: Delegates.SubtitleContentItem {
                subtitle: i18n("Compare a set of emoji on both devices")
                itemDelegate: emojiVerification
            }
            onClicked: root.session.sendStartSas()
            anchors.centerIn: parent
        }
        Message {
            visible: root.session.state === KeyVerificationSession.DONE
            anchors.centerIn: parent
            text: i18n("Successfully verified device **%1**", root.session.remoteDeviceId)
            icon: "security-high"
        }
    }

    footer: QQC2.ToolBar {
        visible: root.session.state === KeyVerificationSession.INCOMING
        QQC2.DialogButtonBox {
            anchors.fill: parent
            Item {
                Layout.fillWidth: true
            }
            QQC2.Button {
                text: i18n("Accept")
                icon.name: "dialog-ok"
                onClicked: root.session.sendReady()
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            }
            QQC2.Button {
                text: i18n("Decline")
                icon.name: "dialog-cancel"
                onClicked: root.session.cancelVerification("m.user", "Declined")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
            }
        }
    }
}
