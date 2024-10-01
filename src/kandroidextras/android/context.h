/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_CONTEXT_H
#define KANDROIDEXTRAS_CONTEXT_H

#include "kandroidextras_export.h"

class QJniObject;

namespace KAndroidExtras
{

/** Methods around android.content.Context. */
namespace Context
{
KANDROIDEXTRAS_EXPORT QJniObject getPackageName();
}

}

#endif // KANDROIDEXTRAS_CONTEXT_H
