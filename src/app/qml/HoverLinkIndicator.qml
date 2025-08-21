// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

RowLayout {
    id: root

    property string text

    onTextChanged: {
        // This is done so the text doesn't disappear for a split second while in the opacity transition
        if (root.text.length > 0) {
            urlLabel.text = root.text
        }
    }

    Kirigami.OverlayZStacking.layer: Kirigami.OverlayZStacking.ToolTip
    z: Kirigami.OverlayZStacking.z
    spacing: 0

    opacity: (!root.text.startsWith("https://matrix.to/") && root.text.length > 0) ? 1 : 0
    visible: opacity > 0

    Behavior on opacity {
        OpacityAnimator {
            duration: Kirigami.Units.shortDuration
            easing.type: Easing.InOutQuad
        }
    }

    QQC2.Control {
        Kirigami.Theme.colorSet: Kirigami.Theme.View

        Accessible.ignored: true

        contentItem: QQC2.Label {
            id: urlLabel

            elide: Text.ElideRight
        }

        background: Kirigami.ShadowedRectangle {
            corners.topRightRadius: Kirigami.Units.cornerRadius
            color: Kirigami.Theme.backgroundColor
            border {
                color: Kirigami.ColorUtils.linearInterpolation(Kirigami.Theme.backgroundColor, Kirigami.Theme.textColor, Kirigami.Theme.frameContrast)
                width: 1
            }
        }
    }
}
