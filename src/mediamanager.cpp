// SPDX-FileCopyrightText: 2023 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "mediamanager.h"

void MediaManager::startPlayback()
{
    Q_EMIT playbackStarted();
}
