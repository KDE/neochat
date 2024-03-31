# SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

qt_add_library(devtools STATIC)
qt_add_qml_module(devtools
    URI org.kde.neochat.devtools
    OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/src/org/kde/neochat/devtools
    QML_FILES
        DevtoolsPage.qml
        AccountData.qml
        FeatureFlagPage.qml
        RoomData.qml
        ServerData.qml
        StateKeys.qml
)
