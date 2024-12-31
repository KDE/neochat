// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2

import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.ApplicationWindow {
    id: root

    title: "Timeline Memory Test"

    minimumWidth: Kirigami.Units.gridUnit * 30
    minimumHeight: Kirigami.Units.gridUnit * 30

    visible: true

    QQC2.ScrollView {
        width: root.width
        height: root.height

        contentItem: ListView {
            model: messageFilterModel

            delegate: EventDelegate {
                room: memTestTimelineModel.room
            }
        }
    }
}
