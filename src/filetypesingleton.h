/* SPDX-FileCopyrightText: 2015 Klaralvdalens Datakonsult AB
 * SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
 * SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL
 */

#pragma once

#include <QObject>
#include <qqml.h>
#include <QMimeDatabase>

class FileTypeSingletonPrivate;

class FileTypeSingleton : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QStringList supportedImageFormats READ supportedImageFormats CONSTANT FINAL)
    Q_PROPERTY(QStringList supportedAnimatedImageFormats READ supportedAnimatedImageFormats CONSTANT FINAL)
    QML_NAMED_ELEMENT(FileType)
    QML_SINGLETON

public:
    explicit FileTypeSingleton(QObject *parent = nullptr);
    ~FileTypeSingleton();

    // Most of the code in this public section was copy/pasted from qmimedatabase.h
    Q_INVOKABLE QMimeType mimeTypeForName(const QString &nameOrAlias) const;

    enum MatchMode {
        MatchDefault,
        MatchExtension,
        MatchContent
    };
    Q_ENUM(MatchMode)

    Q_INVOKABLE QMimeType mimeTypeForFile(const QString &fileName, MatchMode mode = MatchDefault) const;
    Q_INVOKABLE QMimeType mimeTypeForFile(const QFileInfo &fileInfo, MatchMode mode = MatchDefault) const;
    Q_INVOKABLE QList<QMimeType> mimeTypesForFileName(const QString &fileName) const;

    Q_INVOKABLE QMimeType mimeTypeForData(const QByteArray &data) const;
    Q_INVOKABLE QMimeType mimeTypeForData(QIODevice *device) const;

    Q_INVOKABLE QMimeType mimeTypeForUrl(const QUrl &url) const;
    Q_INVOKABLE QMimeType mimeTypeForFileNameAndData(const QString &fileName, QIODevice *device) const;
    Q_INVOKABLE QMimeType mimeTypeForFileNameAndData(const QString &fileName, const QByteArray &data) const;

    Q_INVOKABLE QString suffixForFileName(const QString &fileName) const;

    // These return a list of file extensions, not mimetypes
    QStringList supportedImageFormats() const;
    QStringList supportedAnimatedImageFormats() const;

private:
    const QScopedPointer<FileTypeSingletonPrivate> d_ptr;
    Q_DECLARE_PRIVATE(FileTypeSingleton)
    Q_DISABLE_COPY(FileTypeSingleton)
};

QML_DECLARE_TYPE(FileTypeSingleton)
