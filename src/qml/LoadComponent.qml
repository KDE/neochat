// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

/**
 * @brief A component to show a link preview loading from a message.
 */
RowLayout {
    id: root

    required property int type

    /**
     * @brief Standard height for the link preview.
     *
     * When the content of the link preview is larger than this it will be
     * elided/hidden until maximized.
     */
    property var defaultHeight: Kirigami.Units.gridUnit * 3 + Kirigami.Units.smallSpacing * 2

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    enum Type {
        Reply,
        LinkPreview
    }

    Layout.fillWidth: true
    Layout.maximumWidth: root.maxContentWidth

    Rectangle {
        Layout.fillHeight: true
        width: Kirigami.Units.smallSpacing
        color: Kirigami.Theme.highlightColor
    }
    QQC2.BusyIndicator {}
    Kirigami.Heading {
        Layout.fillWidth: true
        Layout.minimumHeight: root.defaultHeight
        verticalAlignment: Text.AlignVCenter
        level: 2
        text: {
            switch (root.type) {
                case LoadComponent.Reply:
                    return i18n("Loading reply");
                case LoadComponent.LinkPreview:
                    return i18n("Loading URL preview");

            }
        }
    }
}
