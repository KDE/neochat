/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL
 */

#include "filetypesingleton.h"
#include <QImageReader>
#include <QMovie>

static QStringList byteArrayListToStringList(const QByteArrayList &byteArrayList)
{
    QStringList stringList;
    for(const QByteArray &byteArray : byteArrayList) {
        stringList.append(QString::fromLocal8Bit(byteArray));
    }
    return stringList;
}

class FileTypeSingletonPrivate
{
    Q_DECLARE_PUBLIC(FileTypeSingleton)
    Q_DISABLE_COPY(FileTypeSingletonPrivate)
public:
    FileTypeSingletonPrivate(FileTypeSingleton *qq);
    FileTypeSingleton * const q_ptr;
    QMimeDatabase mimetypeDatabase;
    QStringList supportedImageFormats = byteArrayListToStringList(QImageReader::supportedImageFormats());
    QStringList supportedAnimatedImageFormats = byteArrayListToStringList(QMovie::supportedFormats());
};

FileTypeSingletonPrivate::FileTypeSingletonPrivate(FileTypeSingleton* qq) : q_ptr(qq)
{
}

FileTypeSingleton::FileTypeSingleton(QObject* parent)
    : QObject(parent)
    , d_ptr(new FileTypeSingletonPrivate(this))
{
}

FileTypeSingleton::~FileTypeSingleton() noexcept
{
}

QMimeType FileTypeSingleton::mimeTypeForName(const QString& nameOrAlias) const
{
    Q_D(const FileTypeSingleton);
    return d->mimetypeDatabase.mimeTypeForName(nameOrAlias);
}

QMimeType FileTypeSingleton::mimeTypeForFile(const QString& fileName, MatchMode mode) const
{
    Q_D(const FileTypeSingleton);
    return d->mimetypeDatabase.mimeTypeForFile(fileName, static_cast<QMimeDatabase::MatchMode>(mode));
}

QMimeType FileTypeSingleton::mimeTypeForFile(const QFileInfo& fileInfo, MatchMode mode) const
{
    Q_D(const FileTypeSingleton);
    return d->mimetypeDatabase.mimeTypeForFile(fileInfo, static_cast<QMimeDatabase::MatchMode>(mode));
}

QList<QMimeType> FileTypeSingleton::mimeTypesForFileName(const QString& fileName) const
{
    Q_D(const FileTypeSingleton);
    return d->mimetypeDatabase.mimeTypesForFileName(fileName);
}

QMimeType FileTypeSingleton::mimeTypeForData(const QByteArray& data) const
{
    Q_D(const FileTypeSingleton);
    return d->mimetypeDatabase.mimeTypeForData(data);
}

QMimeType FileTypeSingleton::mimeTypeForData(QIODevice* device) const
{
    Q_D(const FileTypeSingleton);
    return d->mimetypeDatabase.mimeTypeForData(device);
}

QMimeType FileTypeSingleton::mimeTypeForUrl(const QUrl& url) const
{
    Q_D(const FileTypeSingleton);
    return d->mimetypeDatabase.mimeTypeForUrl(url);
}

QMimeType FileTypeSingleton::mimeTypeForFileNameAndData(const QString& fileName, QIODevice* device) const
{
    Q_D(const FileTypeSingleton);
    return d->mimetypeDatabase.mimeTypeForFileNameAndData(fileName, device);
}

QMimeType FileTypeSingleton::mimeTypeForFileNameAndData(const QString& fileName, const QByteArray& data) const
{
    Q_D(const FileTypeSingleton);
    return d->mimetypeDatabase.mimeTypeForFileNameAndData(fileName, data);
}

QString FileTypeSingleton::suffixForFileName(const QString& fileName) const
{
    Q_D(const FileTypeSingleton);
    return d->mimetypeDatabase.suffixForFileName(fileName);
}

QStringList FileTypeSingleton::supportedImageFormats() const
{
    Q_D(const FileTypeSingleton);
    return d->supportedImageFormats;
}

QStringList FileTypeSingleton::supportedAnimatedImageFormats() const
{
    Q_D(const FileTypeSingleton);
    return d->supportedAnimatedImageFormats;
}
