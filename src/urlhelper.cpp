// SPDX-FileCopyrightText: 2022 Nicolas Fella <nicolas.fella@gmx.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "urlhelper.h"

#include <QFile>
#include <QtGlobal>

#ifdef Q_OS_ANDROID
#include <QDesktopServices>
#else
#include <KIO/OpenUrlJob>
#endif

// QDesktopServices::openUrl doesn't support XDG activation yet, OpenUrlJob does
// On Android XDG activation is not relevant, so use QDesktopServices::openUrl to avoid the heavy KIO dependency
void UrlHelper::openUrl(const QUrl &url)
{
#ifdef Q_OS_ANDROID
    QDesktopServices::openUrl(url);
#else
    auto *job = new KIO::OpenUrlJob(url);
    job->start();
#endif
}

void UrlHelper::copyTo(const QUrl &origin, const QUrl &destination)
{
    QFile originFile(origin.toLocalFile());
    originFile.copy(destination.toLocalFile());
}

#include "moc_urlhelper.cpp"
