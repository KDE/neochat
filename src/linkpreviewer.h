// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QUrl>

namespace Quotient
{
class RoomMessageEvent;
}

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
     * @brief Whether the there is a link to preview.
     *
     * A linkPreviwer is empty if the URL is empty.
     */
    Q_PROPERTY(bool empty READ empty NOTIFY emptyChanged)

public:
    explicit LinkPreviewer(const NeoChatRoom *room = nullptr, const Quotient::RoomMessageEvent *event = nullptr, QObject *parent = nullptr);

    [[nodiscard]] QUrl url() const;
    [[nodiscard]] bool loaded() const;
    [[nodiscard]] QString title() const;
    [[nodiscard]] QString description() const;
    [[nodiscard]] QUrl imageSource() const;
    [[nodiscard]] bool empty() const;

    /**
     * @brief Whether the given event has at least 1 pre-viewable link.
     *
     * A link is only pre-viewable if it is http, https or something starting with www.
     */
    static bool hasPreviewableLinks(const Quotient::RoomMessageEvent *event);

private:
    const NeoChatRoom *m_currentRoom;
    const Quotient::RoomMessageEvent *m_event;

    bool m_loaded;
    QString m_title = QString();
    QString m_description = QString();
    QUrl m_imageSource = QUrl();
    QUrl m_url;

    void loadUrlPreview();

    /**
     * @brief Return the link to be previewed from the given event.
     *
     * This function is designed to give only links that should be previewed so
     * http, https or something starting with www. The first valid link is returned.
     */
    static QUrl linkPreview(const Quotient::RoomMessageEvent *event);

Q_SIGNALS:
    void loadedChanged();
    void titleChanged();
    void descriptionChanged();
    void imageSourceChanged();
    void urlChanged();
    void emptyChanged();
};
Q_DECLARE_METATYPE(LinkPreviewer *)
