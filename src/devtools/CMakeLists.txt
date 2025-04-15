# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(Devtools STATIC)
ecm_add_qml_module(Devtools GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.devtools
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/devtools
    QML_FILES
        DevtoolsPage.qml
        AccountData.qml
        DebugOptions.qml
        FeatureFlagPage.qml
        RoomData.qml
        ServerData.qml
        StateKeys.qml
    SOURCES
        models/statefiltermodel.cpp
        models/statekeysmodel.cpp
        models/statemodel.cpp
)

target_include_directories(Devtools PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(Devtools PRIVATE
    Qt::Core
    LibNeoChat
)
