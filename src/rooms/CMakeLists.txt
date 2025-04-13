# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(Rooms STATIC)
ecm_add_qml_module(Rooms GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.rooms
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/rooms
    QML_FILES
        RoomListPage.qml
        SpaceDrawer.qml
        RoomDelegate.qml
        RoomTreeSection.qml
        ExploreComponent.qml
        ExploreComponentMobile.qml
        UserInfo.qml
        UserInfoDesktop.qml
        RoomContextMenu.qml
        SpaceListContextMenu.qml
    SOURCES
        models/publicroomlistmodel.cpp
        models/roomtreeitem.cpp
        models/roomtreemodel.cpp
        models/sortfilterroomlistmodel.cpp
        models/sortfilterroomtreemodel.cpp
        models/sortfilterspacelistmodel.cpp
)

ecm_qt_declare_logging_category(Rooms
    HEADER "publicroomlist_logging.h"
    IDENTIFIER "PublicRoomList"
    CATEGORY_NAME "org.kde.neochat.publicroomlistmodel"
    DESCRIPTION "Neochat: publicroomlistmodel"
    DEFAULT_SEVERITY Info
    EXPORT NEOCHAT
)

target_include_directories(Rooms PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(Rooms PRIVATE
    Qt::Core
    Qt::Quick
    KF6::Kirigami
    LibNeoChat
)
