# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(Timeline STATIC)
ecm_add_qml_module(Timeline GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.timeline
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/timeline
    QML_FILES
        TimelineView.qml
        EventDelegate.qml
        HiddenDelegate.qml
        MessageDelegate.qml
        LoadingDelegate.qml
        PredecessorDelegate.qml
        ReadMarkerDelegate.qml
        SpacerDelegate.qml
        StateDelegate.qml
        SuccessorDelegate.qml
        TimelineEndDelegate.qml
        Bubble.qml
        AvatarFlow.qml
        QuickActions.qml
        TypingPane.qml
        DelegateContextMenu.qml
    SOURCES
        messageattached.cpp
        messagedelegate.cpp
        timelinedelegate.cpp
        enums/delegatetype.h
        models/mediamessagefiltermodel.cpp
        models/messagecontentfiltermodel.cpp
        models/messagefiltermodel.cpp
        models/messagemodel.cpp
        models/pinnedmessagemodel.cpp
        models/readmarkermodel.cpp
        models/searchmodel.cpp
        models/timelinemessagemodel.cpp
        models/timelinemodel.cpp
        models/webshortcutmodel.cpp
    DEPENDENCIES
        QtQuick
)

target_include_directories(Timeline PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/enums ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(Timeline PRIVATE
    Qt::Core
    Qt::Quick
    Qt::QuickControls2
    KF6::Kirigami
    LibNeoChat
    MessageContent
)
