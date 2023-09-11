// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#include "linkpreviewer.h"

#include "controller.h"

#include <Quotient/connection.h>
#include <Quotient/csapi/content-repo.h>

#include "neochatconfig.h"
#include "neochatroom.h"

using namespace Quotient;

LinkPreviewer::LinkPreviewer(QObject *parent, const NeoChatRoom *room, const QUrl &url)
    : QObject(parent)
    , m_currentRoom(room)
    , m_loaded(false)
    , m_url(url)
{
    loadUrlPreview();
    if (m_currentRoom) {
        connect(m_currentRoom, &NeoChatRoom::urlPreviewEnabledChanged, this, &LinkPreviewer::loadUrlPreview);
    }
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowLinkPreviewChanged, this, &LinkPreviewer::loadUrlPreview);
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
    if (url != m_url) {
        m_url = url;
        urlChanged();
        loadUrlPreview();
    }
}

void LinkPreviewer::loadUrlPreview()
{
    if (!m_currentRoom || !NeoChatConfig::showLinkPreview() || !m_currentRoom->urlPreviewEnabled()) {
        return;
    }
    if (m_url.scheme() == QStringLiteral("https")) {
        m_loaded = false;
        Q_EMIT loadedChanged();

        auto conn = m_currentRoom->connection();
        GetUrlPreviewJob *job = conn->callApi<GetUrlPreviewJob>(m_url);

        connect(job, &BaseJob::success, this, [this, job, conn]() {
            const auto json = job->jsonData();
            m_title = json["og:title"_ls].toString().trimmed();
            m_description = json["og:description"_ls].toString().trimmed().replace("\n"_ls, " "_ls);

            auto imageUrl = QUrl(json["og:image"_ls].toString());
            if (imageUrl.isValid() && imageUrl.scheme() == QStringLiteral("mxc")) {
                m_imageSource = conn->makeMediaUrl(imageUrl);
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

bool LinkPreviewer::empty() const
{
    return m_url.isEmpty();
}

#include "moc_linkpreviewer.cpp"
