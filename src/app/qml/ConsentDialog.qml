// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

Kirigami.Dialog {
    id: root

    required property string url

    width: Math.min(Kirigami.Units.gridUnit * 24, QQC2.ApplicationWindow.window.width)
    height: Kirigami.Units.gridUnit * 8
    leftPadding: Kirigami.Units.largeSpacing
    rightPadding: Kirigami.Units.largeSpacing

    title: i18nc("@title:dialog", "User Consent")

    contentItem: QQC2.Label {
        text: i18nc("@info", "Your homeserver requires you to agree to its terms and conditions before being able to use it. Please click the button below to read them.")
        wrapMode: Text.WordWrap
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter
    }
    customFooterActions: [
        Kirigami.Action {
            text: i18nc("@action:button", "Open")
            icon.name: "internet-services"
            onTriggered: {
                UrlHelper.openUrl(root.url);
                root.close();
            }
        }
    ]
}
