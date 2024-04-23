// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QUrl>

class NeoChatRoom;

/**
 * @class LinkPreviewer
 *
 * A class to download the link preview info for a URL and provide a QML interface for it.
 *
 * To use set the URL property and then access the other parameters which will be
 * populated once loaded is true.
 */
class LinkPreviewer : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    /**
     * @brief The URL to get the preview for.
     */
    Q_PROPERTY(QUrl url READ url NOTIFY urlChanged)

    /**
     * @brief Whether the preview information has been loaded.
     */
    Q_PROPERTY(bool loaded READ loaded NOTIFY loadedChanged)

    /**
     * @brief The title of the preview.
     */
    Q_PROPERTY(QString title READ title NOTIFY titleChanged)

    /**
     * @brief The description of the preview.
     */
    Q_PROPERTY(QString description READ description NOTIFY descriptionChanged)

    /**
     * @brief The image source for the preview.
     */
    Q_PROPERTY(QUrl imageSource READ imageSource NOTIFY imageSourceChanged)

    /**
     * @brief Whether there is a link to preview.
     *
     * A linkPreviwer is empty if the URL is empty.
     */
    Q_PROPERTY(bool empty READ empty NOTIFY emptyChanged)

public:
    LinkPreviewer() = default;
    explicit LinkPreviewer(const QUrl &url, QObject *parent = nullptr);

    [[nodiscard]] QUrl url() const;
    [[nodiscard]] bool loaded() const;
    [[nodiscard]] QString title() const;
    [[nodiscard]] QString description() const;
    [[nodiscard]] QUrl imageSource() const;
    [[nodiscard]] bool empty() const;

    /**
     * @brief Whether the given string has at least 1 pre-viewable link.
     *
     * A link is only pre-viewable if it is http, https or something starting with www.
     */
    static bool hasPreviewableLinks(const QString &string);

    /**
     * @brief Return previewable links from the given string.
     *
     * This function is designed to give only links that should be previewed so
     * http, https or something starting with www. The first valid link is returned.
     */
    static QList<QUrl> linkPreviews(QString string);

private:
    bool m_loaded;
    QString m_title = QString();
    QString m_description = QString();
    QUrl m_imageSource = QUrl();
    QUrl m_url;

    void loadUrlPreview();

Q_SIGNALS:
    void loadedChanged();
    void titleChanged();
    void descriptionChanged();
    void imageSourceChanged();
    void urlChanged();
    void emptyChanged();
};
Q_DECLARE_METATYPE(LinkPreviewer *)
