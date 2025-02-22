// SPDX-FileCopyrightText: 2025 Joshua Goins <josh@redstrate.com>
// SPDX-License-Identifier: GPL-2.0-or-later

#include "timezonemodel.h"

#include <KLocalizedString>

using namespace Qt::Literals::StringLiterals;

TimeZoneModel::TimeZoneModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_timezoneIds = QTimeZone::availableTimeZoneIds();
}

QVariant TimeZoneModel::data(const QModelIndex &index, int role) const
{
    switch (role) {
    case Qt::DisplayRole: {
        if (index.row() == 0) {
            return i18nc("@item:inlistbox Prefer not to say which timezone im in", "Prefer not to say");
        } else {
            return m_timezoneIds[index.row() - 1];
        }
    }
    default:
        return {};
    }
}

int TimeZoneModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return m_timezoneIds.count() + 1;
}

int TimeZoneModel::indexOfValue(const QString &code)
{
    const auto it = std::ranges::find(std::as_const(m_timezoneIds), code.toUtf8());
    if (it != m_timezoneIds.cend()) {
        return std::distance(m_timezoneIds.cbegin(), it) + 1;
    } else {
        return 0;
    }
}

#include "moc_timezonemodel.cpp"
