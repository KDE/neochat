/*
    SPDX-FileCopyrightText: 2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_CALENDARCONTRACT_H
#define KANDROIDEXTRAS_CALENDARCONTRACT_H

#include <KAndroidExtras/AndroidTypes>
#include <KAndroidExtras/JavaTypes>
#include <KAndroidExtras/JniObject>
#include <KAndroidExtras/JniProperty>

namespace KAndroidExtras
{

/** CalendarContracts.EventColumns wrapper. */
class CalendarColumns
{
    JNI_UNMANAGED_OBJECT(CalendarColumns, android::provider::CalendarContract_CalendarColumns)

    JNI_CONSTANT(jint, CAL_ACCESS_CONTRIBUTOR)
    JNI_CONSTANT(jint, CAL_ACCESS_EDITOR)
    JNI_CONSTANT(jint, CAL_ACCESS_FREEBUSY)
    JNI_CONSTANT(jint, CAL_ACCESS_NONE)
    JNI_CONSTANT(jint, CAL_ACCESS_OVERRIDE)
    JNI_CONSTANT(jint, CAL_ACCESS_OWNER)
    JNI_CONSTANT(jint, CAL_ACCESS_READ)
    JNI_CONSTANT(jint, CAL_ACCESS_RESPOND)
    JNI_CONSTANT(jint, CAL_ACCESS_ROOT)
};

/** CalendarContracts.EventColumns wrapper. */
class EventsColumns
{
    JNI_UNMANAGED_OBJECT(EventsColumns, android::provider::CalendarContract_EventsColumns)

    JNI_CONSTANT(jint, ACCESS_CONFIDENTIAL)
    JNI_CONSTANT(jint, ACCESS_DEFAULT)
    JNI_CONSTANT(jint, ACCESS_PRIVATE)
    JNI_CONSTANT(jint, ACCESS_PUBLIC)

    JNI_CONSTANT(jint, AVAILABILITY_BUSY)
    JNI_CONSTANT(jint, AVAILABILITY_FREE)
    JNI_CONSTANT(jint, AVAILABILITY_TENTATIVE)
};

/** CalendarContracts.AttendeesColumns wrapper. */
class AttendeesColumns
{
    JNI_UNMANAGED_OBJECT(AttendeesColumns, android::provider::CalendarContract_AttendeesColumns)

    JNI_CONSTANT(jint, ATTENDEE_STATUS_ACCEPTED)
    JNI_CONSTANT(jint, ATTENDEE_STATUS_DECLINED)
    JNI_CONSTANT(jint, ATTENDEE_STATUS_INVITED)
    JNI_CONSTANT(jint, ATTENDEE_STATUS_NONE)
    JNI_CONSTANT(jint, ATTENDEE_STATUS_TENTATIVE)

    JNI_CONSTANT(jint, RELATIONSHIP_ATTENDEE)
    JNI_CONSTANT(jint, RELATIONSHIP_NONE)
    JNI_CONSTANT(jint, RELATIONSHIP_ORGANIZER)
    JNI_CONSTANT(jint, RELATIONSHIP_PERFORMER)
    JNI_CONSTANT(jint, RELATIONSHIP_SPEAKER)

    JNI_CONSTANT(jint, TYPE_NONE)
    JNI_CONSTANT(jint, TYPE_OPTIONAL)
    JNI_CONSTANT(jint, TYPE_REQUIRED)
    JNI_CONSTANT(jint, TYPE_RESOURCE)
};

/** CalendarContract.RemindersColumns wrapper. */
class RemindersColumns
{
    JNI_UNMANAGED_OBJECT(RemindersColumns, android::provider::CalendarContract_RemindersColumns)

    JNI_CONSTANT(jint, METHOD_ALARM)
    JNI_CONSTANT(jint, METHOD_ALERT)
    JNI_CONSTANT(jint, METHOD_DEFAULT)
    JNI_CONSTANT(jint, METHOD_EMAIL)
    JNI_CONSTANT(jint, METHOD_SMS)
};

}

#endif // KANDROIDEXTRAS_OPENABLECOLUMNS_H
