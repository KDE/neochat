# SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(LibNeoChat STATIC)

target_sources(LibNeoChat PRIVATE
    neochatconnection.cpp
    neochatroom.cpp
    neochatroommember.cpp
    accountmanager.cpp
    chatbarcache.cpp
    clipboard.cpp
    delegatesizehelper.cpp
    emojitones.cpp
    eventhandler.cpp
    filetransferpseudojob.cpp
    filetype.cpp
    linkpreviewer.cpp
    roomlastmessageprovider.cpp
    spacehierarchycache.cpp
    texthandler.cpp
    urlhelper.cpp
    utils.cpp
    enums/messagecomponenttype.h
    enums/messagetype.h
    enums/powerlevel.cpp
    enums/pushrule.h
    events/imagepackevent.cpp
    events/pollevent.cpp
    models/actionsmodel.cpp
    models/customemojimodel.cpp
    models/emojimodel.cpp
    models/imagepacksmodel.cpp
    models/livelocationsmodel.cpp
    models/locationsmodel.cpp
    models/stickermodel.cpp
)

ecm_add_qml_module(LibNeoChat GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.libneochat
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/libneochat
)

ecm_qt_declare_logging_category(LibNeoChat
    HEADER "eventhandler_logging.h"
    IDENTIFIER "EventHandling"
    CATEGORY_NAME "org.kde.neochat.eventhandler"
    DEFAULT_SEVERITY Info
)

generate_export_header(LibNeoChat BASE_NAME LibNeoChat)
target_include_directories(LibNeoChat PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/enums ${CMAKE_CURRENT_SOURCE_DIR}/events ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(LibNeoChat PUBLIC
    Qt::Core
    Qt::Multimedia
    Qt::Quick
    KF6::ConfigCore
    KF6::CoreAddons
    KF6::I18n
    KF6::Kirigami
    QuotientQt6
    cmark::cmark
    QCoro::Core
    QCoro::Network
)

if(NOT ANDROID)
    target_link_libraries(LibNeoChat PUBLIC
        KF6::KIOWidgets
        ICU::uc
    )
    target_compile_definitions(LibNeoChat PUBLIC -DHAVE_ICU)
endif()
