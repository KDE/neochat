// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "profilefieldshelper.h"
#include "jobs/neochatprofilefieldjobs.h"
#include "neochatconnection.h"

#include <KLocalizedString>

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

QString ProfileFieldsHelper::localTime() const
{
    if (m_timezone.isEmpty()) {
        return {};
    }
    const QTimeZone timeZone(m_timezone.toUtf8());
    const QString localTime = QDateTime::currentDateTime(timeZone).time().toString(QLocale::system().timeFormat(QLocale::ShortFormat));

    QTimeZone::TimeType timeType = QTimeZone::StandardTime;
    if (timeZone.hasDaylightTime() && timeZone.isDaylightTime(QDateTime::currentDateTime(timeZone))) {
        timeType = QTimeZone::DaylightTime;
    }

    const QString timeZoneName = timeZone.displayName(timeType);

    return i18nc("@info:label Local time (timezone offset)", "%1 (%2)", localTime, timeZoneName);
}

bool ProfileFieldsHelper::loading() const
{
    return m_loading;
}

void ProfileFieldsHelper::load()
{
    m_generation++;
    m_loading = false;
    Q_EMIT loadingChanged();

    if (!m_connection || m_userId.isEmpty()) {
        return;
    }

    if (!m_connection->supportsProfileFields()) {
        setLoading(false);
        return;
    }

    setLoading(true);

    const auto generation = m_generation;
    m_connection->callApi<NeoChatGetProfileFieldJob>(BackgroundRequest, m_userId, QStringLiteral("m.tz"))
        .then(
            this,
            [this, generation](const auto &job) {
                if (generation != m_generation) {
                    return;
                }
                m_timezone = job->value();
                Q_EMIT localTimeChanged();
                m_fetchedTimezone = true;
                checkIfFinished();
            },
            [this, generation] {
                if (generation != m_generation) {
                    return;
                }
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
