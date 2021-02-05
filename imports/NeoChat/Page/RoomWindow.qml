/**
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */
import QtQuick 2.12
import QtQuick.Controls 2.12 as QQC2
import QtQuick.Window 2.2
import QtQuick.Layouts 1.12
import org.kde.kirigami 2.14 as Kirigami

Kirigami.ApplicationWindow {
    id: window
    required property var currentRoom
    minimumWidth: Kirigami.Units.gridUnit * 10
    minimumHeight: Kirigami.Units.gridUnit * 15
    pageStack.initialPage: RoomPage {
        visible: true
        currentRoom: window.currentRoom
    }
}
