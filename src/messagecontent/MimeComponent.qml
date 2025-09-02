// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.coreaddons
import org.kde.kirigami as Kirigami

/**
 * @brief A component to show media based upon its mime type.
 */
RowLayout {
    id: root

    property alias mimeIconSource: icon.source
    property alias label: nameLabel.text
    property string subLabel: ""
    property int size: 0
    property int duration: 0

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
            Layout.alignment: caption.visible ? Qt.AlignLeft | Qt.AlignBottom : Qt.AlignLeft | Qt.AlignVCenter

            elide: Text.ElideRight
        }
        QQC2.Label {
            id: caption

            Layout.fillWidth: true

            text: (root.subLabel || root.size || root.duration || '') && [
                root.subLabel,
                root.size && Format.formatByteSize(root.size),
                root.duration > 0 && Format.formatDuration(root.duration),
            ].filter(Boolean).join(" | ")

            elide: Text.ElideRight
            visible: text.length > 0
            opacity: 0.7
        }
    }
}
