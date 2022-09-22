// SPDX-FileCopyrightText: 2022 Bharadwaj Raju <bharadwaj.raju777@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-or-later OR LicenseRef-KDE-Accepted-GPL

#include "linkpreviewer.h"

#include "controller.h"

LinkPreviewer::LinkPreviewer(QObject *parent)
    : QObject(parent)
    , m_loaded(false)
{
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

QString LinkPreviewer::imageSource() const
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

        auto conn = Controller::instance().activeConnection();

        GetUrlPreviewJob *job = conn->callApi<GetUrlPreviewJob>(m_url.toString());

        connect(job, &BaseJob::success, this, [this, job]() {
            const auto json = job->jsonData();
            m_title = json["og:title"].toString();
            m_description = json["og:description"].toString();
            m_imageSource = json["og:image"].toString();
            m_loaded = true;
            Q_EMIT titleChanged();
            Q_EMIT descriptionChanged();
            Q_EMIT imageSourceChanged();
            Q_EMIT loadedChanged();
        });
    }
}
