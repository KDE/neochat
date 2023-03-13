// SPDX-FileCopyrightText: 2022 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2

import org.kde.kirigami 2.15 as Kirigami

Flow {
    id: root

    property var avatarSize: Kirigami.Units.iconSizes.small
    property alias model: avatarFlowRepeater.model
    property string toolTipText

    spacing: -avatarSize / 2
    Repeater {
        id: avatarFlowRepeater
        delegate: Kirigami.Avatar {
            implicitWidth: avatarSize
            implicitHeight: avatarSize

            name: modelData.displayName
            source: modelData.avatarMediaId ? ("image://mxc/" + modelData.avatarMediaId) : ""
            color: modelData.color
        }
    }

    QQC2.ToolTip.text: toolTipText
    QQC2.ToolTip.visible: hoverHandler.hovered
    QQC2.ToolTip.delay: Kirigami.Units.toolTipDelay

    HoverHandler {
        id: hoverHandler
        margin: Kirigami.Units.smallSpacing
    }
}
