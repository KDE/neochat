/**
 * SPDX-FileCopyrightText: 2018 Black Hat <bhat@encom.eu.org>
 * SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12
import QtGraphicalEffects 1.0

Item {
    id: item

    property alias source: mask.source

    Rectangle {
        id: circleMask

        width: parent.width
        height: parent.height

        smooth: true
        visible: false

        radius: Math.max(width/2, height/2)
    }

    OpacityMask {
        id: mask

        width: parent.width
        height: parent.height

        maskSource: circleMask
    }
}
