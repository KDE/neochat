// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick.Layouts 1.15
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.12 as Kirigami

Kirigami.Page {
    title: i18n("Loading…")

    Kirigami.PlaceholderMessage {
        id: loadingIndicator
        anchors.centerIn: parent
        text: i18n("Loading…")
        QQC2.BusyIndicator {
            running: loadingIndicator.visible
            Layout.alignment: Qt.AlignHCenter
        }
    }
}
