/*
 * SPDX-FileCopyrightText: 2017 Atul Sharma <atulsharma406@gmail.com>
 * SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.purpose as Purpose
import org.kde.kirigami as Kirigami
import org.kde.notification

import org.kde.neochat

Kirigami.Page {
    id: root

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    property alias index: jobView.index
    property alias model: jobView.model

    QQC2.Action {
        shortcut: 'Escape'
        onTriggered: root.closeDialog()
    }

    Notification {
        id: sharingFailed
        eventId: "Share"
        text: i18n("Sharing failed")
        urgency: Notification.NormalUrgency
    }

    Notification {
        id: sharingSuccess
        eventId: "Share"
        flags: Notification.Persistent
    }

    Component.onCompleted: {
        jobView.start();
    }

    Purpose.JobView {
        id: jobView

        anchors.fill: parent
        onStateChanged: {
            if (state === Purpose.PurposeJobController.Finished) {
                if (jobView.job?.output?.url?.length > 0) {
                    sharingSuccess.text = i18n("Shared url for image is <a href='%1'>%1</a>", jobView.job.output.url);
                    sharingSuccess.sendEvent();
                    Clipboard.saveText(jobView.job.output.url);
                }
                root.closeDialog();
            } else if (state === Purpose.PurposeJobController.Error) {
                // Show failure notification
                sharingFailed.sendEvent();
                root.closeDialog();
            } else if (state === Purpose.PurposeJobController.Cancelled) {
                // Do nothing
                root.closeDialog();
            }
        }
    }
}
