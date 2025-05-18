# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(Spaces STATIC)
ecm_add_qml_module(Spaces GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.spaces
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/spaces
    QML_FILES
        SpaceHomePage.qml
        SpaceHierarchyDelegate.qml
        RemoveChildDialog.qml
        SelectExistingRoomDialog.qml
    SOURCES
        models/spacechildrenmodel.cpp
        models/spacechildsortfiltermodel.cpp
        models/spacetreeitem.cpp
)

target_include_directories(Spaces PRIVATE ${CMAKE_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_link_libraries(Spaces PRIVATE
    Qt::Core
    Qt::Quick
    KF6::Kirigami
    LibNeoChat
)
