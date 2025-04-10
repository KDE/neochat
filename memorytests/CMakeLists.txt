# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

add_definitions(-DDATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}" )

qt_add_executable(timeline-memtest
    main.cpp
)

target_link_libraries(timeline-memtest PRIVATE neochatplugin Timelineplugin)
target_link_libraries(timeline-memtest PUBLIC
    Qt::Core
    Qt::Quick
    Qt::Qml
    Qt::Gui
    Qt::QuickControls2
    Qt::Widgets
    KF6::I18n
    KF6::Kirigami
    QuotientQt6
    LibNeoChat
    Timeline
)

ecm_add_qml_module(timeline-memtest URI org.kde.neochat.timeline-memtest GENERATE_PLUGIN_SOURCE
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/timeline-memtest
    QML_FILES
        Main.qml
    SOURCES
        memtesttimelinemodel.cpp
        memtesttimelinemodel.h
    DEPENDENCIES
        QtCore
        QtQuick
    IMPORTS
        org.kde.neochat.timeline
)
