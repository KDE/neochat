// SPDX-FileCopyrightText: 2021 Carl Schwan <carlschwan@kde.org>
// SPDX-License-Identifier: LGPL-2.1-or-later

#include <KColorSchemeManager>
#include <QAbstractItemModel>

#include "colorschemer.h"

ColorSchemer::ColorSchemer(QObject *parent)
    : QObject(parent)
    , c(new KColorSchemeManager(this))
{
}

ColorSchemer::~ColorSchemer()
{
}

QAbstractItemModel *ColorSchemer::model() const
{
    return c->model();
}

void ColorSchemer::apply(int idx)
{
    c->activateScheme(c->model()->index(idx, 0));
}

int ColorSchemer::indexForCurrentScheme()
{
    return -1;
    // return c->indexForSchemeId(c->activeSchemeId()).row();
}

#include "moc_colorschemer.cpp"
