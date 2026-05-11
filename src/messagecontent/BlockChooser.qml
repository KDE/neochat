// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

pragma ComponentBehavior: Bound

import QtQuick
import QtQuick.Layouts
import Qt.labs.qmlmodels

import org.kde.neochat
import org.kde.neochat.libneochat as LibNeoChat

/**
 * @brief Select a message component based on a Blocks.Type.
 */
BaseBlockChooser {
    id: root

    DelegateChoice {
        roleValue: Blocks.ThreadBody
        delegate: ThreadBodyComponent {
            onSelectedTextChanged: selectedText => {
                root.selectedTextChanged(selectedText);
            }
            onHoveredLinkChanged: hoveredLink => {
                root.hoveredLinkChanged(hoveredLink);
            }
        }
    }

    DelegateChoice {
        roleValue: Blocks.ChatBar
        delegate: ChatBarCore {
            /**
             * @brief The Blocks::Block for the delegate.
             */
            required property Block block

            Layout.fillWidth: true
            Layout.maximumWidth: Message.maxContentWidth
            room: Message.room
            chatBarType: block.isEditing ? LibNeoChat.ChatBarType.Edit : LibNeoChat.ChatBarType.Thread
            threadRootId: block.threadRootId
            maxAvailableWidth: Message.maxContentWidth

            onCancel: if (chatBarType == LibNeoChat.ChatBarType.Edit) {
                Message.room.cacheForType(LibNeoChat.ChatBarType.Edit).clearRelations();
            } else {
                // Because we override Message with the chat bar's own model and we need the
                // higher level event content model.
                parent.Message.contentModel.cancelReplyInThread();
            }
        }
    }
}
