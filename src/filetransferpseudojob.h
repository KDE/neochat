// SPDX-FileCopyrightText: 2022 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KJob>
#include <QString>

class FileTransferPseudoJob : public KJob
{
public:
    enum Operation {
        Download,
        Upload,
    };
    Q_ENUM(Operation);
    FileTransferPseudoJob(Operation operation, const QString &srcDest, const QString &path);
    void fileTransferProgress(QString id, qint64 progress, qint64 total);
    void fileTransferCompleted(QString id, QUrl localFile);
    void fileTransferFailed(QString id, QString errorMessage = {});

    void start() override;

private:
    QString m_path;
    QString m_eventId;
    Operation m_operation;
};
