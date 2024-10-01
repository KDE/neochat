/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "javalocale.h"

#include <KAndroidExtras/JavaTypes>
#include <KAndroidExtras/JniSignature>

#include <QLocale>

using namespace KAndroidExtras;

QJniObject Locale::fromLocale(const QLocale &locale)
{
    auto lang = QJniObject::fromString(QLocale::languageToString(locale.language()));
    auto country = QJniObject::fromString(QLocale::countryToString(locale.country()));
    auto script = QJniObject::fromString(QLocale::scriptToString(locale.script()));

    return QJniObject(Jni::typeName<java::util::Locale>(),
                      (const char *)Jni::signature<void(java::lang::String, java::lang::String, java::lang::String)>(),
                      lang.object(),
                      country.object(),
                      script.object());
}

QJniObject Locale::current()
{
    return fromLocale(QLocale());
}
