// SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL

#include "filetype.h"
#include <QImageReader>
#include <QMovie>

static QStringList byteArrayListToStringList(const QByteArrayList &byteArrayList)
{
    QStringList stringList;
    for (const QByteArray &byteArray : byteArrayList) {
        stringList.append(QString::fromLocal8Bit(byteArray));
    }
    return stringList;
}

class FileTypePrivate
{
    Q_DECLARE_PUBLIC(FileType)
    Q_DISABLE_COPY(FileTypePrivate)
public:
    FileTypePrivate(FileType *qq);
    FileType *const q_ptr;
    QMimeDatabase mimetypeDatabase;
    QStringList supportedImageFormats = byteArrayListToStringList(QImageReader::supportedImageFormats());
    QStringList supportedAnimatedImageFormats = byteArrayListToStringList(QMovie::supportedFormats());
};

FileTypePrivate::FileTypePrivate(FileType *qq)
    : q_ptr(qq)
{
}

FileType::FileType(QObject *parent)
    : QObject(parent)
    , d_ptr(new FileTypePrivate(this))
{
}

FileType::~FileType() noexcept
{
}

FileType &FileType::instance()
{
    static FileType _instance;
    return _instance;
}

QMimeType FileType::mimeTypeForName(const QString &nameOrAlias) const
{
    Q_D(const FileType);
    return d->mimetypeDatabase.mimeTypeForName(nameOrAlias);
}

QMimeType FileType::mimeTypeForFile(const QString &fileName, MatchMode mode) const
{
    Q_D(const FileType);
    return d->mimetypeDatabase.mimeTypeForFile(fileName, static_cast<QMimeDatabase::MatchMode>(mode));
}

QMimeType FileType::mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode) const
{
    Q_D(const FileType);
    return d->mimetypeDatabase.mimeTypeForFile(fileInfo, static_cast<QMimeDatabase::MatchMode>(mode));
}

QList<QMimeType> FileType::mimeTypesForFileName(const QString &fileName) const
{
    Q_D(const FileType);
    return d->mimetypeDatabase.mimeTypesForFileName(fileName);
}

QMimeType FileType::mimeTypeForData(const QByteArray &data) const
{
    Q_D(const FileType);
    return d->mimetypeDatabase.mimeTypeForData(data);
}

QMimeType FileType::mimeTypeForData(QIODevice *device) const
{
    Q_D(const FileType);
    return d->mimetypeDatabase.mimeTypeForData(device);
}

QMimeType FileType::mimeTypeForUrl(const QUrl &url) const
{
    Q_D(const FileType);
    return d->mimetypeDatabase.mimeTypeForUrl(url);
}

QMimeType FileType::mimeTypeForFileNameAndData(const QString &fileName, QIODevice *device) const
{
    Q_D(const FileType);
    return d->mimetypeDatabase.mimeTypeForFileNameAndData(fileName, device);
}

QMimeType FileType::mimeTypeForFileNameAndData(const QString &fileName, const QByteArray &data) const
{
    Q_D(const FileType);
    return d->mimetypeDatabase.mimeTypeForFileNameAndData(fileName, data);
}

QString FileType::suffixForFileName(const QString &fileName) const
{
    Q_D(const FileType);
    return d->mimetypeDatabase.suffixForFileName(fileName);
}

QStringList FileType::supportedImageFormats() const
{
    Q_D(const FileType);
    return d->supportedImageFormats;
}

QStringList FileType::supportedAnimatedImageFormats() const
{
    Q_D(const FileType);
    return d->supportedAnimatedImageFormats;
}

bool FileType::fileHasImage(const QUrl &file) const
{
    const auto mimeType = mimeTypeForFile(file.toString());
    return mimeType.isValid() && supportedImageFormats().contains(mimeType.preferredSuffix());
}

#include "moc_filetype.cpp"
