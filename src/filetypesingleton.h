// SPDX-FileCopyrightText: 2015 Klaralvdalens Datakonsult AB
// SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QFileInfo>
#include <QMimeDatabase>
#include <QObject>
#include <qqml.h>

class FileTypeSingletonPrivate;

/**
 * @class FileTypeSingleton
 *
 * Provide a singleton to expose the functionality of QMimeDatabase.
 *
 * @sa QMimeDatabase
 */
class FileTypeSingleton : public QObject
{
    Q_OBJECT

    /**
     * @brief List of supported image formats.
     *
     * Returns a list of file extensions, not MIME types.
     */
    Q_PROPERTY(QStringList supportedImageFormats READ supportedImageFormats CONSTANT FINAL)

    /**
     * @brief List of supported animated image formats.
     *
     * Returns a list of file extensions, not MIME types.
     */
    Q_PROPERTY(QStringList supportedAnimatedImageFormats READ supportedAnimatedImageFormats CONSTANT FINAL)

    QML_NAMED_ELEMENT(FileType)
    QML_SINGLETON

public:
    explicit FileTypeSingleton(QObject *parent = nullptr);
    ~FileTypeSingleton();

    /**
     * @brief Returns a MIME type for nameOrAlias or an invalid one if none found.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QMimeType mimeTypeForName(const QString &nameOrAlias) const;

    enum MatchMode { MatchDefault, MatchExtension, MatchContent };
    Q_ENUM(MatchMode)

    /**
     * @brief Returns a MIME type for the file named fileName using mode.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QMimeType mimeTypeForFile(const QString &fileName, FileTypeSingleton::MatchMode mode = MatchDefault) const;

    /**
     * @brief Returns a MIME type for fileInfo.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QMimeType mimeTypeForFile(const QFileInfo &fileInfo, FileTypeSingleton::MatchMode mode = MatchDefault) const;

    /**
     * @brief Returns the MIME types for the file name fileName.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QList<QMimeType> mimeTypesForFileName(const QString &fileName) const;

    /**
     * @brief Returns a MIME type for data.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QMimeType mimeTypeForData(const QByteArray &data) const;

    /**
     * @brief Returns a MIME type for the data in device.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QMimeType mimeTypeForData(QIODevice *device) const;

    /**
     * @brief Returns a MIME type for url.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QMimeType mimeTypeForUrl(const QUrl &url) const;

    /**
     * @brief Returns a MIME type for the given fileName and device data.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QMimeType mimeTypeForFileNameAndData(const QString &fileName, QIODevice *device) const;

    /**
     * @brief Returns a MIME type for the given fileName and device data.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QMimeType mimeTypeForFileNameAndData(const QString &fileName, const QByteArray &data) const;

    /**
     * @brief Returns the suffix for the file fileName, as known by the MIME database.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QString suffixForFileName(const QString &fileName) const;

    QStringList supportedImageFormats() const;
    QStringList supportedAnimatedImageFormats() const;

private:
    const QScopedPointer<FileTypeSingletonPrivate> d_ptr;
    Q_DECLARE_PRIVATE(FileTypeSingleton)
    Q_DISABLE_COPY(FileTypeSingleton)
};

QML_DECLARE_TYPE(FileTypeSingleton)
