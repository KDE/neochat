# SPDX-FileCopyrightText: 2024 Tobias Fella <tobias.fella@kde.org>
# SPDX-License-Identifier: BSD-2-Clause

find_library(LIVEKIT_LIB NAMES livekit_ffi)
find_path(LIVEKIT_INCLUDE_DIR NAMES livekit_ffi.h)

add_library(LiveKit UNKNOWN IMPORTED)
set_target_properties(LiveKit PROPERTIES IMPORTED_LOCATION ${LIVEKIT_LIB})
set_target_properties(LiveKit PROPERTIES INTERFACE_INCLUDE_DIRECTORIES ${LIVEKIT_INCLUDE_DIR})
set(LiveKit_FOUND True)
