// SPDX-FileCopyrightText: 2015 Klaralvdalens Datakonsult AB
// SPDX-FileCopyrightText: 2016 The Qt Company Ltd.
// SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-LGPL

#pragma once

#include <QFileInfo>
#include <QMimeDatabase>
#include <QObject>
#include <QQmlEngine>
#include <qqml.h>

class FileTypePrivate;

/**
 * @class FileTypeSingleton
 *
 * Provide a singleton to expose the functionality of QMimeDatabase.
 *
 * @sa QMimeDatabase
 */
class FileType : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON

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

public:
    ~FileType();
    static FileType &instance();
    static FileType *create(QQmlEngine *engine, QJSEngine *)
    {
        engine->setObjectOwnership(&instance(), QQmlEngine::CppOwnership);
        return &instance();
    }

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
    Q_INVOKABLE QMimeType mimeTypeForFile(const QString &fileName, FileType::MatchMode mode = MatchDefault) const;

    /**
     * @brief Returns a MIME type for fileInfo.
     *
     * @sa QMimeDatabase
     */
    Q_INVOKABLE QMimeType mimeTypeForFile(const QFileInfo &fileInfo, FileType::MatchMode mode = MatchDefault) const;

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

    bool fileHasImage(const QUrl &file) const;

private:
    explicit FileType(QObject *parent = nullptr);

    const QScopedPointer<FileTypePrivate> d_ptr;
    Q_DECLARE_PRIVATE(FileType)
    Q_DISABLE_COPY(FileType)
};
