// SPDX-FileCopyrightText: 2026 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "neochatdatetime.h"

#include <KFormat>

using namespace Qt::Literals::StringLiterals;

NeoChatDateTime::NeoChatDateTime(QDateTime dateTime)
    : m_dateTime(dateTime)
{
}

QDateTime NeoChatDateTime::dateTime() const
{
    return m_dateTime;
}

QString NeoChatDateTime::hourMinuteString() const
{
    return m_dateTime.toLocalTime().toString(u"hh:mm"_s);
}

QString NeoChatDateTime::shortDateTime() const
{
    return QLocale().toString(m_dateTime.toLocalTime(), QLocale::ShortFormat);
}

QString NeoChatDateTime::longDateTime() const
{
    return QLocale().toString(m_dateTime.toLocalTime(), QLocale::LongFormat);
}

QString NeoChatDateTime::relativeDate() const
{
    KFormat formatter;
    return formatter.formatRelativeDate(m_dateTime.toLocalTime().date(), QLocale::ShortFormat);
}

QString NeoChatDateTime::relativeDateTime() const
{
    KFormat formatter;
    return formatter.formatRelativeDateTime(m_dateTime.toLocalTime(), QLocale::ShortFormat);
}

QString NeoChatDateTime::shortRelativeDateTime() const
{
    if (m_dateTime > QDate::currentDate().startOfDay()) {
        return hourMinuteString();
    }
    return relativeDate() + u", "_s + hourMinuteString();
}

bool NeoChatDateTime::isValid() const
{
    return m_dateTime.isValid();
}

bool NeoChatDateTime::operator==(const QDateTime &right) const
{
    return m_dateTime == right;
}

#include "moc_neochatdatetime.cpp"
