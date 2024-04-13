// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import QtQml

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.neochat

Kirigami.Page {
    id: root

    title: i18n("Session Verification")

    required property var session

    states: [
        State {
            name: "cancelled"
            when: root.session.state === KeyVerificationSession.CANCELED
            PropertyChanges {
                target: stateLoader
                sourceComponent: verificationCanceled
            }
        },
        State {
            name: "waitingForVerification"
            when: root.session.state === KeyVerificationSession.WAITINGFORVERIFICATION
            PropertyChanges {
                target: stateLoader
                sourceComponent: emojiSas
            }
        },
        State {
            name: "waitingForReady"
            when: root.session.state === KeyVerificationSession.WAITINGFORREADY
            PropertyChanges {
                target: stateLoader
                sourceComponent: message
            }
        },
        State {
            name: "incoming"
            when: root.session.state === KeyVerificationSession.INCOMING
            PropertyChanges {
                target: stateLoader
                sourceComponent: message
            }
        },
        State {
            name: "waitingForMac"
            when: root.session.state === KeyVerificationSession.WAITINGFORMAC
            PropertyChanges {
                target: stateLoader
                sourceComponent: message
            }
        },
        State {
            name: "ready"
            when: root.session.state === KeyVerificationSession.READY
            PropertyChanges {
                target: stateLoader
                sourceComponent: emojiVerificationComponent
            }
        },
        State {
            name: "done"
            when: root.session.state === KeyVerificationSession.DONE
            PropertyChanges {
                target: stateLoader
                sourceComponent: message
            }
        }
    ]

    Loader {
        id: stateLoader
        anchors.fill: parent
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

    Component {
        id: verificationCanceled
        VerificationCanceled {
            reason: root.session.error
        }
    }

    Component {
        id: emojiSas
        EmojiSas {
            model: root.session.sasEmojis
            onReject: root.session.cancelVerification(KeyVerificationSession.MISMATCHED_SAS)
            onAccept: root.session.sendMac()
        }
    }

    Component {
        id: message
        Message {
            icon: {
                switch (root.session.state) {
                case KeyVerificationSession.WAITINGFORREADY:
                case KeyVerificationSession.INCOMING:
                case KeyVerificationSession.WAITINGFORMAC:
                    return "security-medium-symbolic";
                case KeyVerificationSession.DONE:
                    return "security-high";
                default:
                    return "";
                }
            }
            text: {
                switch (root.session.state) {
                case KeyVerificationSession.WAITINGFORREADY:
                    return i18n("Waiting for device to accept verification.");
                case KeyVerificationSession.INCOMING:
                    return i18n("Incoming key verification request from device **%1**", root.session.remoteDeviceId);
                case KeyVerificationSession.WAITINGFORMAC:
                    return i18n("Waiting for other party to verify.");
                case KeyVerificationSession.DONE:
                    return i18n("Successfully verified device **%1**", root.session.remoteDeviceId)
                default:
                    return "";
                }
            }
        }
    }

    Component {
        id: emojiVerificationComponent
        Delegates.RoundedItemDelegate {
            id: emojiVerification
            text: i18n("Emoji Verification")
            contentItem: Delegates.SubtitleContentItem {
                subtitle: i18n("Compare a set of emoji on both devices")
                itemDelegate: emojiVerification
            }
            onClicked: root.session.sendStartSas()
        }
    }
}
