// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick.Layouts 1.15
import QtQuick.Controls 2.12 as QQC2
import org.kde.kirigami 2.19 as Kirigami

Kirigami.Page {
    Kirigami.LoadingPlaceholder {
        id: loadingIndicator
        anchors.centerIn: parent
    }
}
