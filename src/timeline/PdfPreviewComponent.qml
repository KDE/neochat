// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.neochat

Rectangle {
    id: root

    /**
     * @brief FileTransferInfo for any downloading files.
     */
    required property var fileTransferInfo

    /**
     * @brief The attributes of the component.
     */
    required property var componentAttributes

    /**
     * @brief The maximum width that the bubble's content can be.
     */
    property real maxContentWidth: -1

    Layout.preferredWidth: mediaSizeHelper.currentSize.width
    Layout.preferredHeight: mediaSizeHelper.currentSize.height

    color: "white"

    Image {
        anchors.fill: root
        source: root?.fileTransferInfo.localPath ?? ""

        MediaSizeHelper {
            id: mediaSizeHelper
            contentMaxWidth: root.maxContentWidth
            mediaWidth: root.componentAttributes.size.width
            mediaHeight: root.componentAttributes.size.height
        }
    }
}
