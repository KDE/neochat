/**
 * SPDX-FileCopyrightText: 2018-2019 Black Hat <bhat@encom.eu.org>
 *
 * SPDX-License-Identifier: GPL-3.0-only
 */
import QtQuick 2.12

import NeoChat.Setting 1.0

MouseArea {
    signal primaryClicked()
    signal secondaryClicked()

    acceptedButtons: Qt.LeftButton | Qt.RightButton

    onClicked: mouse.button == Qt.RightButton ? secondaryClicked() : primaryClicked()
    onPressAndHold: secondaryClicked()
}
