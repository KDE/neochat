// SPDX-FileCopyrightText: 2021 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "screencast.h"
#include "pipelinemanager.h"
#include <QtCore/QByteArray>

#ifdef Q_OS_LINUX
#include "xscreencast.h"
#endif

ScreenCastManager::ScreenCastManager()
{
#ifdef Q_OS_LINUX
    if (qgetenv("XDG_SESSION_TYPE") == QByteArrayLiteral("wayland")) {
        qDebug() << "This is wayland";
        // TODO wayland backend
    } else if (qgetenv("XDG_SESSION_TYPE") == QByteArrayLiteral("x11")) {
        qDebug() << "This is x11";
        m_screencast = new XScreenCast(this);
    } else {
        qDebug() << "Unknown linux environment";
    }
#endif
}

AbstractScreenCast::AbstractScreenCast(QObject *parent)
    : QObject(parent)
{
}

void ScreenCastManager::requestScreenCast(int index)
{
    if (m_screencast) {
        PipelineManager::instance().add(m_screencast.value()->request(index));
    }
}
