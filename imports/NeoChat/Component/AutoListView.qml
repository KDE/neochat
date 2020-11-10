/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12

ListView {
    pixelAligned: true

    ScrollHelper {
        anchors.fill: parent

        flickable: parent
    }
}
