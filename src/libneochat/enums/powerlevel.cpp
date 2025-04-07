// SPDX-FileCopyrightText: 2024 James Graham <james.h.graham@protonmail.com>
// SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

#include "powerlevel.h"

QString PowerLevel::nameForLevel(Level level)
{
    switch (level) {
    case PowerLevel::Member:
        return i18n("Member");
    case PowerLevel::Moderator:
        return i18n("Moderator");
    case PowerLevel::Admin:
        return i18n("Admin");
    case PowerLevel::Mute:
        return i18n("Mute");
    case PowerLevel::Custom:
        return i18n("Custom");
    default:
        return {};
    }
}

int PowerLevel::valueForLevel(Level level)
{
    switch (level) {
    case PowerLevel::Member:
        return 0;
    case PowerLevel::Moderator:
        return 50;
    case PowerLevel::Admin:
        return 100;
    case PowerLevel::Mute:
        return -1;
    default:
        return {};
    }
}

PowerLevel::Level PowerLevel::levelForValue(int value)
{
    switch (value) {
    case 0:
        return PowerLevel::Member;
    case 50:
        return PowerLevel::Moderator;
    case 100:
        return PowerLevel::Admin;
    case -1:
        return PowerLevel::Mute;
    default:
        return PowerLevel::Custom;
    }
}

PowerLevelModel::PowerLevelModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

bool PowerLevelModel::showMute() const
{
    return m_showMute;
}

void PowerLevelModel::setShowMute(bool showMute)
{
    if (showMute == m_showMute) {
        return;
    }
    m_showMute = showMute;
    Q_EMIT showMuteChanged();
}

QVariant PowerLevelModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return {};
    }
    if (index.row() >= rowCount()) {
        qDebug() << "PowerLevelModel, something's wrong: index.row() >= m_rules.count()";
        return {};
    }

    const auto level = static_cast<PowerLevel::Level>(index.row());
    if (role == NameRole) {
        return i18nc("%1 is the name of the power level, e.g. admin and %2 is the value that represents.",
                     "%1 (%2)",
                     PowerLevel::nameForLevel(level),
                     PowerLevel::valueForLevel(level));
    }
    if (role == ValueRole) {
        return PowerLevel::valueForLevel(level);
    }
    return {};
}

int PowerLevelModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return PowerLevel::NUMLevels - (m_showMute ? 0 : 1);
}

QHash<int, QByteArray> PowerLevelModel::roleNames() const
{
    return {{NameRole, "name"}, {ValueRole, "value"}};
}

#include "moc_powerlevel.cpp"
