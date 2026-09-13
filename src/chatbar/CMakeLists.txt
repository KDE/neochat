# SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(Chatbar STATIC)
ecm_add_qml_module(Chatbar GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.chatbar
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/chatbar
    QML_FILES
        ChatBar.qml
        ChatBarCore.qml
        ChatBarFeatures.qml
        RichEditBar.qml
        SendBar.qml
        CompletionMenu.qml
        EmojiDelegate.qml
        EmojiGrid.qml
        PieProgressBar.qml
        EmojiPicker.qml
        EmojiDialog.qml
        EmojiTonesPicker.qml
        StylePicker.qml
        StyleDelegate.qml
        ImageEditorPage.qml
        VoiceMessageDialog.qml
        LinkDialog.qml
        LocationChooser.qml
        NewPollDialog.qml
        TableDialog.qml
        StyleButton.qml
    SOURCES
        actionsmodel.cpp
        chatbuttonhelper.cpp
        completionmodel.cpp
        completionlist.cpp
        completionproxymodel.cpp
        styledelegatehelper.cpp
)

ecm_qt_declare_logging_category(LibNeoChat
    HEADER "chatbarlogging.h"
    IDENTIFIER "ChatBar"
    CATEGORY_NAME "org.kde.neochat.chatbar"
    DEFAULT_SEVERITY Info
)

target_include_directories(Chatbar PRIVATE ${CMAKE_BINARY_DIR})
target_link_libraries(Chatbar PRIVATE
    Qt::Core
    Qt::Quick
    Qt::QuickControls2
    KF6::Kirigami
    LibNeoChat
)
