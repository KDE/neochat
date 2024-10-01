/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JAVATYPES_H
#define KANDROIDEXTRAS_JAVATYPES_H

#include <KAndroidExtras/JniTypeTraits>
#include <KAndroidExtras/JniTypes>

namespace KAndroidExtras
{

JNI_TYPE(java, io, File)
JNI_TYPE(java, lang, String)
JNI_TYPE(java, util, Locale)

JNI_DECLARE_CONVERTER(java::lang::String, QString, (value.toString()), (QJniObject::fromString(value)))

}

#endif // KANDROIDEXTRAS_JAVATYPES_H
