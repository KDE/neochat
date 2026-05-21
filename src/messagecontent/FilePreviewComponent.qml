// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.coreaddons
import org.kde.kirigami as Kirigami

import org.kde.neochat

/**
 * @brief A component to show a message that has been replied to.
 *
 * Similar to the main timeline delegate a reply delegate is chosen based on the type
 * of message being replied to. The main difference is that not all messages can be
 * show in their original form and are instead visualised with a MIME type delegate
 * e.g. Videos.
 */
Loader {
    id: root

    /**
     * @brief The reaction model to get the reactions from.
     */
    required property FilePreviewBlock block
    Connections {
        target: block

        function onPreviewBlockChanged(): void {
            console.warn("change")
            if (root.block.previewBlock) {
                console.warn("preview changed", root.block.previewBlock.type)
                setSource(root.block.componentSource, {block: root.block.previewBlock})
            }
        }
    }

    onBlockChanged: if (block && block.previewBlock) {
        console.warn("preview changed", root.block.previewBlock.type)
        let properties = {block: block.previewBlock};
        if (block.previewBlock.type === Blocks.Code) {
            properties.editable = false;
            properties.eventId = "";
            properties.author = null;
            properties.index = 0;
            properties.dateTime = undefined;
            properties.currentFocus = false;
        }
        setSource(block.componentSource, properties)
    }

    Layout.fillWidth: true
    Layout.maximumWidth: Message.maxContentWidth

    onItemChanged: console.warn(item, item.block.type)
}
