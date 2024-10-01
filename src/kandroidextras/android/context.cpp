/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "context.h"

#include <KAndroidExtras/JavaTypes>
#include <KAndroidExtras/JniSignature>

#include <QCoreApplication>

using namespace KAndroidExtras;

QJniObject Context::getPackageName()
{
    const QJniObject context = QNativeInterface::QAndroidApplication::context();
    return context.callObjectMethod("getPackageName", Jni::signature<java::lang::String()>());
}
