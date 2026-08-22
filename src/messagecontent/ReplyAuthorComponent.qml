// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.labs.components as KirigamiComponents

import org.kde.neochat

RowLayout {
    id: root

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    implicitHeight: Math.max(replyAvatar.implicitHeight, replyName.implicitHeight)
    spacing: Kirigami.Units.largeSpacing

    KirigamiComponents.Avatar {
        id: replyAvatar

        implicitWidth: Kirigami.Units.iconSizes.small
        implicitHeight: Kirigami.Units.iconSizes.small

        source: Message.contentModel?.author.avatarUrl ?? ""
        name: Message.contentModel?.author.displayName ?? ""
        color: Message.contentModel?.author.color ?? Kirigami.Theme.highlightColor
        asynchronous: true
    }
    QQC2.Label {
        id: replyName
        Layout.fillWidth: true

        color: Message.contentModel?.author.color ?? Kirigami.Theme.highlightColor
        text: Message.contentModel?.author.disambiguatedName ?? ""
        elide: Text.ElideRight
        textFormat: Text.PlainText
    }
}
