# SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(RoomInfo STATIC)
ecm_add_qml_module(RoomInfo GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.roominfo
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/roominfo
    QML_FILES
        RoomDrawer.qml
        RoomDrawerPage.qml
        RoomInformation.qml
        RoomMedia.qml
        DirectChatDrawerHeader.qml
        LocationsPage.qml
        RoomPinnedMessagesPage.qml
        RoomSearchPage.qml
    SOURCES
        locationhelper.cpp
)

target_link_libraries(RoomInfo PRIVATE
    Qt::Core
)
