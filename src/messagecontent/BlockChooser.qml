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
            id: core

            /**
             * @brief The matrix ID of the message event.
             */
            required property string eventId

            /**
             * @brief The Blocks::Block for the delegate.
             */
            required property ChatBarBlock block

            Layout.fillWidth: true
            Layout.maximumWidth: Message.maxContentWidth

            cache: block.cache
            features {
                sendAttachment: false
                sendLocation: false
                sendPoll: false
                sendVoice: false
                cancel: true
            }
            room: Message.room
            maxAvailableWidth: Message.maxContentWidth

            onSend: {
                postHelper.postMessage()
                parent.Message.contentModel.cancelChatBar();
            }
            onCancel: parent.Message.contentModel.cancelChatBar();

            PostMessageHelper {
                id: postHelper
                room: core.Message.room
                cache: block.cache
                editId: core.eventId
                threadRootId: block.threadRootId
            }
        }
    }
}
