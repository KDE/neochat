# SPDX-FileCopyrightText: 2025 James Graham <james.h.graham@protonmail.com>
# SPDX-License-Identifier: BSD-2-Clause

if (NOT ANDROID AND NOT WIN32 AND NOT APPLE AND NOT NEOCHAT_FLATPAK AND NOT NEOCHAT_APPIMAGE)
    add_subdirectory(purpose)
endif()

add_subdirectory(libneochat)
add_subdirectory(login)
add_subdirectory(rooms)
add_subdirectory(roominfo)
add_subdirectory(messagecontent)
add_subdirectory(timeline)
add_subdirectory(spaces)
add_subdirectory(chatbar)
add_subdirectory(settings)
add_subdirectory(devtools)
add_subdirectory(app)
