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
            when: root.session.state === KeyVerificationSession.TRANSITIONED && root.session.sasState === KeyVerificationSession.KEYSEXCHANGED
            PropertyChanges {
                target: stateLoader
                sourceComponent: emojiSas
            }
        },
        State {
            name: "waitingForReady"
            when: root.session.state === KeyVerificationSession.CREATED
            PropertyChanges {
                target: stateLoader
                sourceComponent: message
            }
        },
        State {
            name: "incoming"
            when: root.session.state === KeyVerificationSession.REQUESTED
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
            when: /*root.session.state === KeyVerificationSession.TRANSITIONED && */root.session.sasState === KeyVerificationSession.SASDONE
            PropertyChanges {
                target: stateLoader
                sourceComponent: message
            }
        },
        State {
            name: "confirmed"
            when: root.session.state === KeyVerificationSession.TRANSITIONED && root.session.sasState === KeyVerificationSession.CONFIRMED
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
        visible: root.session.state === KeyVerificationSession.REQUESTED
        QQC2.DialogButtonBox {
            anchors.fill: parent
            Item {
                Layout.fillWidth: true
            }
            QQC2.Button {
                text: i18n("Accept")
                icon.name: "dialog-ok"
                onClicked: root.session.accept()
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
            onAccept: root.session.confirm()
        }
    }

    Component {
        id: message
        Message {
            icon: {
                switch (root.session.state) {
                case KeyVerificationSession.CREATED:
                case KeyVerificationSession.REQUESTED:
                case KeyVerificationSession.WAITINGFORMAC:
                    return "security-medium-symbolic";
                case KeyVerificationSession.DONE:
                    return "security-high";
                case KeyVerificationSession.TRANSITIONED: {
                    if (root.session.sasState === KeyVerificationSession.CONFIRMED) {
                        return "security-high";
                    }
                    if (root.session.sasState === KeyVerificationSession.SASDONE) {
                        return "security-high";
                    }
                }
                default:
                    return "";
                }
            }
            text: {
                switch (root.session.state) {
                case KeyVerificationSession.CREATED:
                    return i18n("Waiting for device to accept verification.");
                case KeyVerificationSession.REQUESTED: {
                    if (root.session.remoteDeviceId.length > 0) {
                        return i18n("Incoming key verification request from device **%1**", root.session.remoteDeviceId);
                    } else {
                        return i18n("Incoming key verification request from **%1**", root.session.remoteUserId);
                    }
                }
                case KeyVerificationSession.WAITINGFORMAC:
                    return i18n("Waiting for other party to verify.");
                case KeyVerificationSession.DONE: {
                    if (root.session.remoteDeviceId.length > 0) {
                        return i18n("Successfully verified device **%1**", root.session.remoteDeviceId)
                    } else {
                        return i18nc("@info", "Successfully verified **%1**", root.session.remoteUserId)
                    }
                }
                case KeyVerificationSession.TRANSITIONED: {
                    if (root.session.sasState === KeyVerificationSession.CONFIRMED) {
                        return i18nc("@info", "Waiting for remote party to confirm verification");
                    }
                    if (root.session.sasState === KeyVerificationSession.SASDONE) {
                        if (root.session.remoteDeviceId.length > 0) {
                            return i18n("Successfully verified device **%1**", root.session.remoteDeviceId)
                        } else {
                            return i18nc("@info", "Successfully verified **%1**", root.session.remoteUserId)
                        }
                    }
                }
                default:
                    return "invalid";
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
