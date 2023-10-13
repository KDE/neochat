// SPDX-FileCopyrightText: 1997 Matthias Kalle Dalheimer <kalle@kde.org>
// SPDX-FileCopyrightText: 2002 Holger Freyther <freyther@kde.org>
// SPDX-FileCopyrightText: 2008 Volker Krause <vkrause@kde.org>
// SPDX-FileCopyrightText: 2023 Tobias Fella <fella@posteo.de>
// SPDX-License-Identifier: LGPL-2.0-or-later

#include "logger.h"

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QLoggingCategory>
#include <QMutex>
#include <QStandardPaths>

static QLoggingCategory::CategoryFilter oldCategoryFilter = nullptr;
static QtMessageHandler oldHandler = nullptr;
static bool e2eeDebugEnabled = false;

class FileDebugStream : public QIODevice
{
    Q_OBJECT
public:
    FileDebugStream()
        : mType(QtCriticalMsg)
    {
        open(WriteOnly);
    }

    bool isSequential() const override
    {
        return true;
    }
    qint64 readData(char *, qint64) override
    {
        return 0;
    }
    qint64 readLineData(char *, qint64) override
    {
        return 0;
    }

    qint64 writeData(const char *data, qint64 len) override
    {
        if (!mFileName.isEmpty()) {
            QFile outputFile(mFileName);
            outputFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Unbuffered);
            outputFile.write(data, len);
            outputFile.putChar('\n');
            outputFile.close();
        }

        return len;
    }

    void setFileName(const QString &fileName)
    {
        mFileName = fileName;
    }

    void setType(QtMsgType type)
    {
        mType = type;
    }

private:
    QString mFileName;
    QtMsgType mType;
};

class DebugPrivate
{
public:
    DebugPrivate()
        : origHandler(nullptr)
    {
    }

    ~DebugPrivate()
    {
        qInstallMessageHandler(origHandler);
        file.close();
    }

    void log(QtMsgType type, const QMessageLogContext &context, const QString &message)
    {
        QMutexLocker locker(&mutex);
        QByteArray buf;
        QTextStream str(&buf);
        str << QDateTime::currentDateTime().toString(Qt::ISODate) << QStringLiteral(" [");
        switch (type) {
        case QtDebugMsg:
            str << QStringLiteral("DEBUG");
            break;
        case QtInfoMsg:
            str << QStringLiteral("INFO ");
            break;
        case QtWarningMsg:
            str << QStringLiteral("WARN ");
            break;
        case QtFatalMsg:
            str << QStringLiteral("FATAL");
            break;
        case QtCriticalMsg:
            str << QStringLiteral("CRITICAL");
            break;
        }
        str << QStringLiteral("] ") << context.category << QStringLiteral(": ");
        if (context.file && *context.file && context.line) {
            str << context.file << QStringLiteral(":") << context.line << QStringLiteral(": ");
        }
        if (context.function && *context.function) {
            str << context.function << QStringLiteral(": ");
        }
        str << message << QStringLiteral("\n");
        str.flush();
        file.write(buf.constData(), buf.size());
        file.flush();

        if (oldHandler && (strcmp(context.category, "quotient.e2ee") != 0 || e2eeDebugEnabled)) {
            oldHandler(type, context, message);
        }
    }

    void setName(const QString &appName)
    {
        name = appName;

        if (file.isOpen()) {
            file.close();
        }
        const auto &filePath = QStringLiteral("%1%2%3").arg(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation), QDir::separator(), appName);

        QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator());
        auto entryList = dir.entryList({appName + QStringLiteral(".*")});
        std::sort(entryList.begin(), entryList.end(), [](const auto &left, const auto &right) {
            auto leftIndex = left.split(QStringLiteral(".")).last().toInt();
            auto rightIndex = right.split(QStringLiteral(".")).last().toInt();
            return leftIndex > rightIndex;
        });
        for (const auto &entry : entryList) {
            bool ok = false;
            const auto index = entry.split(QStringLiteral(".")).last().toInt(&ok);
            if (!ok) {
                continue;
            }
            QFileInfo info(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + entry);
            if (info.exists()) {
                QFile file(info.absoluteFilePath());
                if (index > 50) {
                    file.remove();
                    continue;
                }
                const auto &newName = QStringLiteral("%1.%2").arg(filePath, QString::number(index + 1));
                const auto success = file.copy(newName);
                if (success) {
                    file.remove();
                } else {
                    qFatal("Cannot rename log file '%s' to '%s': %s",
                           qUtf8Printable(file.fileName()),
                           qUtf8Printable(newName),
                           qUtf8Printable(file.errorString()));
                }
            }
        }

        QFileInfo finfo(filePath);
        if (!finfo.absoluteDir().exists()) {
            QDir().mkpath(finfo.absolutePath());
        }
        file.setFileName(filePath + QStringLiteral(".0"));
        file.open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    }

    void setOrigHandler(QtMessageHandler origHandler_)
    {
        origHandler = origHandler_;
    }

    QMutex mutex;
    QFile file;
    QString name;
    QtMessageHandler origHandler;
    QByteArray loggingCategory;
};

Q_GLOBAL_STATIC(DebugPrivate, sInstance)

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &message)
{
    switch (type) {
    case QtDebugMsg:
    case QtInfoMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
        sInstance()->log(type, context, message);
        break;
    case QtFatalMsg:
        sInstance()->log(QtInfoMsg, context, message);
        abort();
    }
}

void filter(QLoggingCategory *category)
{
    if (qstrcmp(category->categoryName(), "quotient.e2ee") == 0) {
        category->setEnabled(QtDebugMsg, true);
    } else if (oldCategoryFilter) {
        oldCategoryFilter(category);
    }
}

void initLogging()
{
    e2eeDebugEnabled = QLoggingCategory("quotient.e2ee", QtInfoMsg).isEnabled(QtDebugMsg);
    oldCategoryFilter = QLoggingCategory::installFilter(filter);
    oldHandler = qInstallMessageHandler(messageHandler);
    sInstance->setOrigHandler(oldHandler);
    sInstance->setName(QStringLiteral("neochat.log"));
}

#include "logger.moc"
