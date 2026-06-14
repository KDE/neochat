// SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Layouts

import org.kde.neochat

Rectangle {
    id: root

    /**
     * @brief The Blocks::Block for the delegate.
     */
    required property ImageBlock block

    Layout.preferredWidth: mediaSizeHelper.currentSize.width
    Layout.preferredHeight: mediaSizeHelper.currentSize.height

    color: "white"

    Image {
        anchors.fill: root
        source: root.block.fileTransferInfo?.localPath ?? ""

        MediaSizeHelper {
            id: mediaSizeHelper
            contentMaxWidth: root.Message.maxContentWidth
            mediaWidth: root.block.info.pixelSize.width
            mediaHeight: root.block.info.pixelSize.height
        }
    }
}
