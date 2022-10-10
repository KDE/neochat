// SPDX-FileCopyrightText: 2020 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-3.0-only

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.19 as Kirigami

import org.kde.neochat 1.0
import NeoChat.Component 1.0

Kirigami.LoadingPlaceholder {
    property var showContinueButton: false
    property var showBackButton: false
    text: i18n("Synchronizing with your homeserver…")
    icon.name: "cloud-download"

    anchors.centerIn: parent

    explanation: i18n("Please wait. This might take a little while.")
}
