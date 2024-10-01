/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "uri.h"

#include <KAndroidExtras/JavaTypes>
#include <KAndroidExtras/JniSignature>

using namespace KAndroidExtras;

QJniObject Uri::fromUrl(const QUrl &url)
{
    return QJniObject::callStaticObjectMethod(Jni::typeName<android::net::Uri>(),
                                              "parse",
                                              Jni::signature<android::net::Uri(java::lang::String)>(),
                                              QJniObject::fromString(url.toString(QUrl::FullyEncoded)).object<jstring>());
}

QUrl Uri::toUrl(const QJniObject &uri)
{
    if (!uri.isValid()) {
        return QUrl();
    }
    return QUrl(uri.callObjectMethod("toString", Jni::signature<java::lang::String()>()).toString());
}
