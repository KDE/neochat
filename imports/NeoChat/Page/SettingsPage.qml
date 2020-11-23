/**
 * SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

import QtQuick 2.14
import QtQuick.Controls 2.14 as QQC2
import QtQuick.Layouts 1.14

import org.kde.kirigami 2.12 as Kirigami

import org.kde.neochat 1.0

Kirigami.ScrollablePage {
    title: i18n("Settings")

    Kirigami.FormLayout {
        QQC2.CheckBox {
            id: showNotifications
            Kirigami.FormData.label: i18n("Show Notifications:")
            checked: Config.showNotifications
            onToggled: Config.showNotifications = checked
        }
        QQC2.RadioButton {
            Kirigami.FormData.label: i18n("Rooms and private chats:")
            text: i18n("Separated")
            checked: Config.mergeRoomList
            onToggled: Config.mergeRoomList = true
        }
        QQC2.RadioButton {
            text: i18n("Intermixed")
            checked: !Config.mergeRoomList
            onToggled: Config.mergeRoomList = false
        }
    }
}
