/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "activity.h"

#include <KAndroidExtras/AndroidTypes>
#include <KAndroidExtras/Intent>
#include <KAndroidExtras/JniSignature>

#include "private/qandroidextras_p.h"
#include <QCoreApplication>
#include <QJniEnvironment>

using namespace KAndroidExtras;

Intent Activity::getIntent()
{
    const QJniObject activity = QNativeInterface::QAndroidApplication::context();
    if (!activity.isValid())
        return {};

    const auto intent = activity.callObjectMethod("getIntent", Jni::signature<android::content::Intent()>());
    return Intent(Jni::fromHandle<Intent>(intent));
}

bool Activity::startActivity(const Intent &intent, int receiverRequestCode)
{
    QJniEnvironment jniEnv;
    QtAndroidPrivate::startActivity(intent, receiverRequestCode);
    if (jniEnv->ExceptionCheck()) {
        jniEnv->ExceptionClear();
        return false;
    }
    return true;
}
