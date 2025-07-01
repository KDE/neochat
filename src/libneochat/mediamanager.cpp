// SPDX-FileCopyrightText: 2023-2025 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "mediamanager.h"

#include <Quotient/qt_connection_util.h>

void MediaManager::startPlayback()
{
    Q_EMIT playbackStarted();
}

MediaManager::MediaManager()
    : QObject(nullptr)
{
}

#include "moc_mediamanager.cpp"
