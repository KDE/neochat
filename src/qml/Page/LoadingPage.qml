// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-2.0-or-later

import QtQuick.Layouts
import org.kde.kirigami as Kirigami

Kirigami.Page {
    title: i18n("Loading…")
    Kirigami.LoadingPlaceholder {
        id: loadingIndicator
        anchors.centerIn: parent
    }
}
