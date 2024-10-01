/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_SETTINGS_H
#define KANDROIDEXTRAS_SETTINGS_H

#include <KAndroidExtras/AndroidTypes>
#include <KAndroidExtras/JniProperty>

namespace KAndroidExtras
{

/** Methods around android.provider.Settings. */
class Settings
{
    JNI_UNMANAGED_OBJECT(Settings, android::provider::Settings)
    JNI_CONSTANT(java::lang::String, ACTION_APP_NOTIFICATION_SETTINGS)
    JNI_CONSTANT(java::lang::String, ACTION_CHANNEL_NOTIFICATION_SETTINGS)
    JNI_CONSTANT(java::lang::String, EXTRA_APP_PACKAGE)
    JNI_CONSTANT(java::lang::String, EXTRA_CHANNEL_ID)
};

}

#endif // KANDROIDEXTRAS_SETTINGS_H
