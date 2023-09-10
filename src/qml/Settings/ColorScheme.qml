// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts

import org.kde.kirigamiaddons.formcard as FormCard

import org.kde.neochat

FormCard.FormComboBoxDelegate {
    id: root

    text: i18n("Color theme")
    textRole: "display"
    valueRole: "display"
    model: ColorSchemer.model
    Component.onCompleted: currentIndex = ColorSchemer.indexForScheme(Config.colorScheme);
    onCurrentValueChanged: {
        ColorSchemer.apply(currentIndex);
        Config.colorScheme = ColorSchemer.nameForIndex(currentIndex);
        Config.save();
    }
}
