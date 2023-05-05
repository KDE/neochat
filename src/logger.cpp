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
        str << QDateTime::currentDateTime().toString(Qt::ISODate) << " [";
        switch (type) {
        case QtDebugMsg:
            str << "DEBUG";
            break;
        case QtInfoMsg:
            str << "INFO ";
            break;
        case QtWarningMsg:
            str << "WARN ";
            break;
        case QtFatalMsg:
            str << "FATAL";
            break;
        case QtCriticalMsg:
            str << "CRITICAL";
            break;
        }
        str << "] " << context.category << ": ";
        if (context.file && *context.file && context.line) {
            str << context.file << ":" << context.line << ": ";
        }
        if (context.function && *context.function) {
            str << context.function << ": ";
        }
        str << message << "\n";
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
        auto filePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QDir::separator() + appName;

        QFileInfo infoOld(filePath + QLatin1String(".old"));
        if (infoOld.exists()) {
            QFile fileOld(infoOld.absoluteFilePath());
            const bool success = fileOld.remove();
            if (!success) {
                qFatal("Cannot remove old log file '%s': %s", qUtf8Printable(fileOld.fileName()), qUtf8Printable(fileOld.errorString()));
            }
        }

        QFileInfo info(filePath);
        if (info.exists()) {
            QFile file(info.absoluteFilePath());
            const QString oldName = filePath + QLatin1String(".old");
            const bool success = file.copy(oldName);
            if (!success) {
                qFatal("Cannot rename log file '%s' to '%s': %s", qUtf8Printable(file.fileName()), qUtf8Printable(oldName), qUtf8Printable(file.errorString()));
            }
        }

        QFileInfo finfo(filePath);
        if (!finfo.absoluteDir().exists()) {
            QDir().mkpath(finfo.absolutePath());
        }
        file.setFileName(filePath);
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
    e2eeDebugEnabled = QLoggingCategory("quotient.e2ee", QtDebugMsg).isEnabled(QtDebugMsg);
    oldCategoryFilter = QLoggingCategory::installFilter(filter);
    oldHandler = qInstallMessageHandler(messageHandler);
    sInstance->setOrigHandler(oldHandler);
    sInstance->setName("neochat.log");
}

#include "logger.moc"
