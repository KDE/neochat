// SPDX-FileCopyrightText: 2019 Black Hat <bhat@encom.eu.org>
// SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15

import org.kde.kirigami 2.15 as Kirigami

Kirigami.Heading {
    level: 4
    text: model.showSection ? section : ""
    color: Kirigami.Theme.disabledTextColor
    horizontalAlignment: Text.AlignHCenter
    verticalAlignment: Text.AlignVCenter
    topPadding: Kirigami.Units.largeSpacing * 2
    bottomPadding: Kirigami.Units.smallSpacing
}
