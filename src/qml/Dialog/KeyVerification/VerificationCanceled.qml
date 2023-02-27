// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQml 2.15

import org.kde.kirigami 2.19 as Kirigami
import org.kde.neochat 1.0

Message {
    id: verificationCanceled

    required property int reason

    anchors.centerIn: parent
    icon: "security-low"
    text: {
        switch(verificationCanceled.reason) {
            case KeyVerificationSession.NONE:
                return i18n("The session verification was canceled for unknown reason.");
            case KeyVerificationSession.TIMEOUT:
                return i18n("The session verification timed out.");
            case KeyVerificationSession.REMOTE_TIMEOUT:
                return i18n("The session verification timed out for remote party.");
            case KeyVerificationSession.USER:
                return i18n("You canceled the session verification.");
            case KeyVerificationSession.REMOTE_USER:
                return i18n("The remote party canceled the session verification.");
            case KeyVerificationSession.UNEXPECTED_MESSAGE:
                return i18n("The session verification was canceled because we received an unexpected message.");
            case KeyVerificationSession.REMOTE_UNEXPECTED_MESSAGE:
                return i18n("The remote party canceled the session verification because it received an unexpected message.");
            case KeyVerificationSession.UNKNOWN_TRANSACTION:
                return i18n("The session verification was canceled because it received a message for an unknown session.");
            case KeyVerificationSession.REMOTE_UNKNOWN_TRANSACTION:
                return i18n("The remote party canceled the session verification because it received a message for an unknown session.");
            case KeyVerificationSession.UNKNOWN_METHOD:
                return i18n("The session verification was canceled because NeoChat is unable to handle this verification method.");
            case KeyVerificationSession.REMOTE_UNKNOWN_METHOD:
                return i18n("The remote party canceled the session verification because it is unable to handle this verification method.");
            case KeyVerificationSession.KEY_MISMATCH:
                return i18n("The session verification was canceled because the keys are incorrect.");
            case KeyVerificationSession.REMOTE_KEY_MISMATCH:
                return i18n("The remote party canceled the session verification because the keys are incorrect.");
            case KeyVerificationSession.USER_MISMATCH:
                return i18n("The session verification was canceled because it verifies an unexpected user.");
            case KeyVerificationSession.REMOTE_USER_MISMATCH:
                return i18n("The remote party canceled the session verification because it verifies an unexpected user.");
            case KeyVerificationSession.INVALID_MESSAGE:
                return i18n("The session verification was canceled because we received an invalid message.");
            case KeyVerificationSession.REMOTE_INVALID_MESSAGE:
                return i18n("The remote party canceled the session verification because it received an invalid message.");
            case KeyVerificationSession.SESSION_ACCEPTED:
                return i18n("The session was accepted on a different device"); //TODO this should not be visible
            case KeyVerificationSession.REMOTE_SESSION_ACCEPTED:
                return i18n("The session was accepted on a different device"); //TODO neither should this
            case KeyVerificationSession.MISMATCHED_COMMITMENT:
                return i18n("The session verification was canceled because of a mismatched key.");
            case KeyVerificationSession.REMOTE_MISMATCHED_COMMITMENT:
                return i18n("The remote party canceled the session verification because of a mismatched key.");
            case KeyVerificationSession.MISMATCHED_SAS:
                return i18n("The session verification was canceled because the keys do not match.");
            case KeyVerificationSession.REMOTE_MISMATCHED_SAS:
                return i18n("The remote party canceled the session verification because the keys do not match.");
            default:
                return i18n("The session verification was canceled due to an unknown error.");
        }
    }
}
