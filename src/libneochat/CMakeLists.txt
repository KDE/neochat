# SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-3-Clause

add_library(LibNeoChat STATIC)

target_sources(LibNeoChat PRIVATE
    delegatesizehelper.cpp
)

ecm_add_qml_module(LibNeoChat GENERATE_PLUGIN_SOURCE
    URI org.kde.neochat.libneochat
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/libneochat
)

generate_export_header(LibNeoChat BASE_NAME LibNeoChat)
target_link_libraries(LibNeoChat PUBLIC
    Qt::Core
    Qt::Quick
)
