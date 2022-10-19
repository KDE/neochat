// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0

QQC2.ComboBox {
    textRole: "display"
    model: ColorSchemer.model
    Component.onCompleted: currentIndex = ColorSchemer.indexForScheme(Config.colorScheme);
    onActivated: {
        ColorSchemer.apply(currentIndex);
        Config.colorScheme = ColorSchemer.nameForIndex(currentIndex);
        Config.save();
    }
}
