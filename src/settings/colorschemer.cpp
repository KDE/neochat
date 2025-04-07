// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <KColorSchemeManager>
#include <QAbstractItemModel>

#include "colorschemer.h"

ColorSchemer::ColorSchemer(QObject *parent)
    : QObject(parent)
{
    KColorSchemeManager::instance();
}

ColorSchemer::~ColorSchemer()
{
}

QAbstractItemModel *ColorSchemer::model() const
{
    return KColorSchemeManager::instance()->model();
}

void ColorSchemer::apply(int idx)
{
    KColorSchemeManager::instance()->activateScheme(KColorSchemeManager::instance()->model()->index(idx, 0));
}

int ColorSchemer::indexForCurrentScheme()
{
    return KColorSchemeManager::instance()->indexForSchemeId(KColorSchemeManager::instance()->activeSchemeId()).row();
}

#include "moc_colorschemer.cpp"
