/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "contentresolver.h"
#include "openablecolumns.h"
#include "uri.h"

#include <KAndroidExtras/AndroidTypes>
#include <KAndroidExtras/JniSignature>

#include <QCoreApplication>
#include <QString>
#include <QUrl>

using namespace KAndroidExtras;

QJniObject ContentResolver::get()
{
    const QJniObject context = QNativeInterface::QAndroidApplication::context();
    return context.callObjectMethod("getContentResolver", Jni::signature<android::content::ContentResolver()>());
}

QString ContentResolver::mimeType(const QUrl &url)
{
    auto cs = ContentResolver::get();
    const auto uri = Uri::fromUrl(url);
    auto mt = cs.callObjectMethod("getType", Jni::signature<java::lang::String(android::net::Uri)>(), uri.object<jobject>());
    return mt.toString();
}

QString ContentResolver::fileName(const QUrl &url)
{
    auto cs = ContentResolver::get();
    const auto uri = Uri::fromUrl(url);

    // query(Uri uri, String[] projection, String selection, String[] selectionArgs, String sortOrder)
    auto cursor = cs.callObjectMethod(
        "query",
        Jni::signature<android::database::Cursor(android::net::Uri, java::lang::String[], java::lang::String, java::lang::String[], java::lang::String)>(),
        uri.object<jobject>(),
        0,
        0,
        0,
        0);

    const QJniObject DISPLAY_NAME = OpenableColumns::DISPLAY_NAME;
    const auto nameIndex = cursor.callMethod<jint>("getColumnIndex", (const char *)Jni::signature<int(java::lang::String)>(), DISPLAY_NAME.object());
    cursor.callMethod<jboolean>("moveToFirst", (const char *)Jni::signature<bool()>());
    return cursor.callObjectMethod("getString", Jni::signature<java::lang::String(int)>(), nameIndex).toString();
}
