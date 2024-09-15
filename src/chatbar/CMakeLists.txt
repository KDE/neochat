# SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(chatbar STATIC)
ecm_add_qml_module(chatbar GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.chatbar
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/chatbar
    QML_FILES
        AttachDialog.qml
        ChatBar.qml
        CompletionMenu.qml
        EmojiDelegate.qml
        EmojiGrid.qml
        PieProgressBar.qml
        EmojiPicker.qml
        EmojiDialog.qml
        EmojiTonesPicker.qml
)
