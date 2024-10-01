/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "intent.h"
#include "uri.h"

#include <KAndroidExtras/JniArray>
#include <KAndroidExtras/JniSignature>

#include <QUrl>

using namespace KAndroidExtras;

Intent::~Intent() = default;

Intent::operator QJniObject() const
{
    return jniHandle();
}

template<typename T>
QJniObject Intent::getObjectExtra(const char *methodName, const QJniObject &name) const
{
    return jniHandle().callObjectMethod(methodName, Jni::signature<T(java::lang::String)>(), name.object());
}

QString Intent::getStringExtra(const QJniObject &name) const
{
    return getObjectExtra<java::lang::String>("getStringExtra", name).toString();
}

QStringList Intent::getStringArrayExtra(const QJniObject &name) const
{
    const auto extra = getObjectExtra<Jni::Array<java::lang::String>>("getStringArrayExtra", name);
    return Jni::fromArray<QStringList>(extra);
}
