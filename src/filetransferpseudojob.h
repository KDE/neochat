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
public:
    enum Operation {
        Download,
        Upload,
    };
    Q_ENUM(Operation);
    FileTransferPseudoJob(Operation operation, const QString &srcDest, const QString &path);

    /**
     * @brief Set the current number of bytes transferred.
     */
    void fileTransferProgress(QString id, qint64 progress, qint64 total);

    /**
     * @brief Set the file transfer as complete.
     */
    void fileTransferCompleted(QString id, QUrl localFile);

    /**
     * @brief Set the file transfer as failed.
     */
    void fileTransferFailed(QString id, QString errorMessage = {});

    /**
     * @brief Start the file transfer.
     */
    void start() override;

private:
    QString m_path;
    QString m_eventId;
    Operation m_operation;
};
