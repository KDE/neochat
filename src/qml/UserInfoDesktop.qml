// SPDX-FileCopyrightText: 2023 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts

import org.kde.kirigami as Kirigami

import org.kde.neochat

QQC2.ToolBar {
    id: root

    /**
     * @brief The connection for the current user.
     */
    required property NeoChatConnection connection

    padding: 0

    contentItem: ColumnLayout {
        spacing: 0
        Kirigami.Separator {
            Layout.fillWidth: true
        }
        UserInfo {
            bottomEdge: true
            connection: root.connection
        }
    }
}
