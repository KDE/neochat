// SPDX-FileCopyrightText: 2026 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtTest

import org.kde.neochat
import org.kde.neochat.server

TestCase {
    id: root
    width: 500
    height: 300

    ChatBar {
        id: chatBar

        width: root.width
        height: root.height
        currentRoom: null
    }

    SignalSpy {
        id: roomCreatedSpy
        target: accountManager.activeConnection
        signalName: "newRoom"
    }

    function test_f() {
        let id = Server.createRoom("@user:localhost:1234")
        console.warn(accountManager.activeConnection)
        console.warn(accountManager.activeConnection.room(id))
        roomCreatedSpy.wait(1000)
        let room = accountManager.activeConnection.room(id)
        console.warn(room)
        root.verify(false)
    }
}