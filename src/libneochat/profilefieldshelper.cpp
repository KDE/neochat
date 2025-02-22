// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "profilefieldshelper.h"
#include "jobs/neochatprofilefieldjobs.h"
#include "neochatconnection.h"

#include <Quotient/csapi/profile.h>

using namespace Quotient;

NeoChatConnection *ProfileFieldsHelper::connection() const
{
    return m_connection.get();
}

void ProfileFieldsHelper::setConnection(NeoChatConnection *connection)
{
    if (m_connection != connection) {
        m_connection = connection;
        Q_EMIT connectionChanged();
        load();
    }
}

QString ProfileFieldsHelper::userId() const
{
    return m_userId;
}

void ProfileFieldsHelper::setUserId(const QString &id)
{
    if (m_userId != id) {
        m_userId = id;
        Q_EMIT userIdChanged();
        load();
    }
}

QString ProfileFieldsHelper::timezone() const
{
    if (m_timezone.isEmpty()) {
        return {};
    }
    return QTimeZone(m_timezone.toUtf8()).displayName(QTimeZone::GenericTime);
}

bool ProfileFieldsHelper::loading() const
{
    return m_loading;
}

void ProfileFieldsHelper::load()
{
    if (!m_connection || m_userId.isEmpty()) {
        return;
    }

    if (!m_connection->supportsProfileFields()) {
        setLoading(false);
        return;
    }

    setLoading(true);

    m_connection->callApi<NeoChatGetProfileFieldJob>(BackgroundRequest, m_userId, QStringLiteral("m.tz"))
        .then(
            [this](const auto &job) {
                m_timezone = job->value();
                Q_EMIT timezoneChanged();
                m_fetchedTimezone = true;
                checkIfFinished();
            },
            [this] {
                m_fetchedTimezone = true;
                checkIfFinished();
            });
}

void ProfileFieldsHelper::checkIfFinished()
{
    if (m_fetchedTimezone) {
        setLoading(false);
    }
}

void ProfileFieldsHelper::setLoading(const bool loading)
{
    m_loading = loading;
    Q_EMIT loadingChanged();
}
