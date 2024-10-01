/*
    SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_MAKE_QTANDROIDEXTRAS_P_H
#define KANDROIDEXTRAS_MAKE_QTANDROIDEXTRAS_P_H

#include <QJniObject>

/** Mock object for QtAndroid namespace. */
namespace QtAndroidPrivate
{
inline void startActivity(const QJniObject &, int)
{
}
}

#endif
