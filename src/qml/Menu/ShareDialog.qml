/*
 * SPDX-FileCopyrightText: 2017 Atul Sharma <atulsharma406@gmail.com>
 * SPDX-FileCopyrightText: 2021 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.15 as QQC2
import org.kde.purpose 1.0 as Purpose
import org.kde.notification 1.0
import org.kde.kirigami 2.14 as Kirigami

Kirigami.Page {
    id: window

    leftPadding: 0
    rightPadding: 0
    topPadding: 0
    bottomPadding: 0

    property alias index: jobView.index
    property alias model: jobView.model

    QQC2.Action {
        shortcut: 'Escape'
        onTriggered: window.closeDialog()
    }

    Notification {
        id: sharingFailed
        eventId: "sharingFailed"
        text: i18n("Sharing failed")
        urgency: Notification.NormalUrgency
    }

    Notification {
        id: sharingSuccess
        eventId: "sharingSuccess"
        flags: Notification.Persistent
    }

    Component.onCompleted: jobView.start()

    contentItem: Purpose.JobView {
        id: jobView
        onStateChanged: {
            if (state === Purpose.PurposeJobController.Finished) {
                if (jobView.job.output.url !== "") {
                    // Show share url
                    // TODO no needed anymore in purpose > 5.90
                    sharingSuccess.text = i18n("Shared url for image is <a href='%1'>%1</a>", jobView.output.url);
                    sharingSuccess.sendEvent();
                    Clipboard.saveText(jobView.output.url);
                }
                window.closeDialog()
            } else if (state === Purpose.PurposeJobController.Error) {
                // Show failure notification
                sharingFailed.sendEvent();

                window.closeDialog()
            } else if (state === Purpose.PurposeJobController.Cancelled) {
                // Do nothing
                window.closeDialog()
            }
        }
    }
}
