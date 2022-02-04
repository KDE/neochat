// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0

Kirigami.PlaceholderMessage {
    property var showContinueButton: false
    property var showBackButton: false
    property string title: i18n("Loading…")

    anchors.centerIn: parent

    QQC2.Label {
        text: i18n("Please wait. This might take a little while.")
    }

    QQC2.BusyIndicator {
        Layout.alignment: Qt.AlignHCenter
    }
}
