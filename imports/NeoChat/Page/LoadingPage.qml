/**
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
import org.kde.kirigami 2.12 as Kirigami
import QtQuick.Controls 2.12 as QQC2

Kirigami.Page {
    title: i18n("Loading")

    QQC2.BusyIndicator {
        anchors.centerIn: parent
    }
}
