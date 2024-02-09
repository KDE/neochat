# SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

kcoreaddons_add_plugin(neochatplugin SOURCES purposeplugin.cpp INSTALL_NAMESPACE "kf6/purpose")
target_link_libraries(neochatplugin
    Qt::DBus
    KF6::Purpose
    KF6::KIOGui
)
