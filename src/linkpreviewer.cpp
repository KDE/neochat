// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#include "linkpreviewer.h"

#include <connection.h>
#include <csapi/content-repo.h>

#include "neochatroom.h"

using namespace Quotient;

LinkPreviewer::LinkPreviewer(QObject *parent)
    : QObject(parent)
    , m_loaded(false)
{
}

NeoChatRoom *LinkPreviewer::room() const
{
    return m_currentRoom;
}

void LinkPreviewer::setRoom(NeoChatRoom *room)
{
    if (room == m_currentRoom) {
        return;
    }
    m_currentRoom = room;
    Q_EMIT roomChanged();
}

bool LinkPreviewer::loaded() const
{
    return m_loaded;
}

QString LinkPreviewer::title() const
{
    return m_title;
}

QString LinkPreviewer::description() const
{
    return m_description;
}

QUrl LinkPreviewer::imageSource() const
{
    return m_imageSource;
}

QUrl LinkPreviewer::url() const
{
    return m_url;
}

void LinkPreviewer::setUrl(QUrl url)
{
    if (url.scheme() == QStringLiteral("https")) {
        m_loaded = false;
        Q_EMIT loadedChanged();

        m_url = url;
        Q_EMIT urlChanged();

        auto conn = m_currentRoom->connection();
        GetUrlPreviewJob *job = conn->callApi<GetUrlPreviewJob>(m_url.toString());

        connect(job, &BaseJob::success, this, [this, job, conn]() {
            const auto json = job->jsonData();
            m_title = json["og:title"].toString().trimmed();
            m_description = json["og:description"].toString().trimmed().replace("\n", " ");

            auto imageUrl = QUrl(json["og:image"].toString());
            if (imageUrl.isValid() && imageUrl.scheme() == QStringLiteral("mxc")) {
#ifdef QUOTIENT_07
                m_imageSource = conn->makeMediaUrl(imageUrl);
#else
                QUrlQuery q(imageUrl.query());
                q.addQueryItem(QStringLiteral("user_id"), conn->userId());
                imageUrl.setQuery(q);
                m_imageSource = imageUrl;
#endif
            } else {
                m_imageSource = QUrl();
            }

            m_loaded = true;
            Q_EMIT titleChanged();
            Q_EMIT descriptionChanged();
            Q_EMIT imageSourceChanged();
            Q_EMIT loadedChanged();
        });
    }
}
