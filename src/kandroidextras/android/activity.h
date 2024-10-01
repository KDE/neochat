/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_ACTIVITY_H
#define KANDROIDEXTRAS_ACTIVITY_H

#include "kandroidextras_export.h"

namespace KAndroidExtras
{

class Intent;

/** Methods around android.app.Activity. */
namespace Activity
{
/** Returns the Intent that started the activity. */
KANDROIDEXTRAS_EXPORT Intent getIntent();

/** Same as QtAndroid::startActivity(), but with exception handling. */
KANDROIDEXTRAS_EXPORT bool startActivity(const Intent &intent, int receiverRequestCode); // TODO add callback arg
}

}

#endif // KANDROIDEXTRAS_ACTIVITY_H
