// SPDX-FileCopyrightText: 2022 Tobias Fella <tobias.fella@kde.org>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KJob>
#include <QString>

/**
 * @class FileTransferPseudoJob
 *
 * A class inherited from KJob to track a file download.
 *
 * @sa KJob
 */
class FileTransferPseudoJob : public KJob
{
    Q_OBJECT
public:
    enum Operation {
        Download,
        Upload,
    };
    Q_ENUM(Operation)
    FileTransferPseudoJob(Operation operation, const QString &srcDest, const QString &path);

    /**
     * @brief Set the current number of bytes transferred.
     */
    void fileTransferProgress(const QString &id, qint64 progress, qint64 total);

    /**
     * @brief Set the file transfer as complete.
     */
    void fileTransferCompleted(const QString &id, const QUrl &localFile);

    /**
     * @brief Set the file transfer as failed.
     */
    void fileTransferFailed(const QString &id, const QString &errorMessage = {});

    /**
     * @brief Set the file transfer as canceled.
     */
    void fileTransferCanceled(const QString &id);

    /**
     * @brief Start the file transfer.
     */
    void start() override;

protected:
    bool doKill() override;

Q_SIGNALS:
    void cancelRequested(const QString &id);

private:
    QString m_path;
    QString m_eventId;
    Operation m_operation;
};
