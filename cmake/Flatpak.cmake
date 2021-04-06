# SPDX-FileCopyrightText: 2020 Carl Schwan <carl@carlschwan.eu>
# SPDX-License-Identifier: BSD-2-Clause

include(GNUInstallDirs)

# Include FontConfig config which uses the Emoji One font from the
# KDE Flatpak SDK.
install(
    FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/cmake/Flatpak/99-noto-mono-color-emoji.conf
    DESTINATION
        ${CMAKE_INSTALL_SYSCONFDIR}/fonts/conf.d/
)

