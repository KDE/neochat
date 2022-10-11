// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.15

import org.kde.kirigami 2.15 as Kirigami

RowLayout {
    property alias mimeIconSource: icon.source
    property alias label: nameLabel.text
    property alias subLabel: subLabel.text

    spacing: Kirigami.Units.largeSpacing

    Kirigami.Icon {
        id: icon

        fallback: "unknown"
    }
    ColumnLayout {
        Layout.alignment: Qt.AlignVCenter
        Layout.fillWidth: true

        spacing: 0

        QQC2.Label {
            id: nameLabel

            Layout.fillWidth: true
            Layout.alignment: subLabel.visible ? Qt.AlignLeft | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter

            elide: Text.ElideRight
        }
        QQC2.Label {
            id: subLabel

            Layout.fillWidth: true

            elide: Text.ElideRight
            visible: text.length > 0
            opacity: 0.7
        }
    }
}
