/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_ANDROIDTYPES_H
#define KANDROIDEXTRAS_ANDROIDTYPES_H

#include <KAndroidExtras/JniTypes>

namespace KAndroidExtras
{

JNI_TYPE(android, content, ContentResolver)
JNI_TYPE(android, content, Context)
JNI_TYPE(android, content, Intent)
JNI_TYPE(android, database, Cursor)
JNI_NESTED_TYPE(android, Manifest, permission)
JNI_TYPE(android, net, Uri)
JNI_NESTED_TYPE(android, provider, CalendarContract, AttendeesColumns)
JNI_NESTED_TYPE(android, provider, CalendarContract, CalendarColumns)
JNI_NESTED_TYPE(android, provider, CalendarContract, EventsColumns)
JNI_NESTED_TYPE(android, provider, CalendarContract, RemindersColumns)
JNI_TYPE(android, provider, OpenableColumns)
JNI_TYPE(android, provider, Settings)

}

#endif // KANDROIDEXTRAS_ANDROIDTYPES_H
