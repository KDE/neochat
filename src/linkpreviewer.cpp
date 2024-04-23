// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#include "linkpreviewer.h"

#include <Quotient/connection.h>
#include <Quotient/csapi/content-repo.h>
#include <Quotient/events/roommessageevent.h>

#include "neochatconfig.h"
#include "neochatconnection.h"
#include "utils.h"

using namespace Quotient;

LinkPreviewer::LinkPreviewer(const QUrl &url, QObject *parent)
    : QObject(parent)
    , m_loaded(false)
    , m_url(url)
{
    Q_ASSERT(dynamic_cast<Connection *>(this->parent()));

    connect(this, &LinkPreviewer::urlChanged, this, &LinkPreviewer::emptyChanged);
    connect(NeoChatConfig::self(), &NeoChatConfig::ShowLinkPreviewChanged, this, &LinkPreviewer::loadUrlPreview);

    loadUrlPreview();
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

void LinkPreviewer::loadUrlPreview()
{
    if (m_url.scheme() == QStringLiteral("https")) {
        m_loaded = false;
        Q_EMIT loadedChanged();

        auto conn = dynamic_cast<Connection *>(this->parent());
        if (conn == nullptr) {
            return;
        }
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

QList<QUrl> LinkPreviewer::linkPreviews(QString string)
{
    auto data = string.remove(TextRegex::removeRichReply);
    auto linksMatch = TextRegex::url.globalMatch(data);
    QList<QUrl> links;
    while (linksMatch.hasNext()) {
        auto link = linksMatch.next().captured();
        if (!link.contains(QStringLiteral("matrix.to")) && !links.contains(QUrl(link))) {
            links += QUrl(link);
        }
    }
    return links;
}

bool LinkPreviewer::hasPreviewableLinks(const QString &string)
{
    return !linkPreviews(string).isEmpty();
}

#include "moc_linkpreviewer.cpp"
