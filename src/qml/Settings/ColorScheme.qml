// SPDX-FileCopyrightText: 2020 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami
import org.kde.kirigamiaddons.labs.mobileform 0.1 as MobileForm

import org.kde.neochat 1.0

MobileForm.FormComboBoxDelegate {
    Layout.fillWidth: true
    text: i18n("Themes")
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
