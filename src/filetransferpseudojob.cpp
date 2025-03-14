// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "filetransferpseudojob.h"
#include <KLocalizedString>
#include <QDebug>
#include <QUrl>

FileTransferPseudoJob::FileTransferPseudoJob(Operation operation, const QString &path, const QString &eventId)
    : KJob()
    , m_path(path)
    , m_eventId(eventId)
    , m_operation(operation)
{
    setCapabilities(KJob::Killable);
}

void FileTransferPseudoJob::fileTransferProgress(const QString &id, qint64 progress, qint64 total)
{
    if (id != m_eventId) {
        return;
    }
    setProcessedAmount(Unit::Bytes, progress);
    setTotalAmount(Unit::Bytes, total);
}

void FileTransferPseudoJob::fileTransferCompleted(const QString &id, const QUrl &localFile)
{
    Q_UNUSED(localFile);
    if (id != m_eventId) {
        return;
    }
    emitResult();
}

void FileTransferPseudoJob::fileTransferFailed(const QString &id, const QString &errorMessage)
{
    if (id != m_eventId) {
        return;
    }
    setErrorText(errorMessage);
    emitResult();
}

void FileTransferPseudoJob::fileTransferCanceled(const QString &id)
{
    if (id != m_eventId) {
        return;
    }
    setError(KJob::KilledJobError);
    emitResult();
}

void FileTransferPseudoJob::start()
{
    setTotalAmount(Unit::Files, 1);
    Q_EMIT description(this,
                       m_operation == Download ? i18nc("Job heading, like 'Copying'", "Downloading") : i18nc("Job heading, like 'Copying'", "Uploading"),
                       {i18nc("The URL being downloaded/uploaded", "Source"), m_path},
                       {i18nc("The location being downloaded to", "Destination"), m_path});
}

bool FileTransferPseudoJob::doKill()
{
    Q_EMIT cancelRequested(m_eventId);
    return true;
}
