/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_INTENT_H
#define KANDROIDEXTRAS_INTENT_H

#include "kandroidextras_export.h"

#include <KAndroidExtras/AndroidTypes>
#include <KAndroidExtras/JavaTypes>
#include <KAndroidExtras/JniMethod>
#include <KAndroidExtras/JniProperty>
#include <KAndroidExtras/Uri>

class QUrl;

namespace KAndroidExtras
{

/** Methods to interact with android.content.Intent objects.
 *  This does not only offer features beyond what QAndroidIntent, it also provides
 *  a putExtra() implementation that actually interoperates with system services.
 */
class KANDROIDEXTRAS_EXPORT Intent
{
    JNI_OBJECT(Intent, android::content::Intent)
public:
    /** Creates a new empty intent. */
    JNI_CONSTRUCTOR(Intent)
    ~Intent();

    /** Add a category to the intent. */
    JNI_METHOD(android::content::Intent, addCategory, java::lang::String)
    /** Add flags to this intent. */
    JNI_METHOD(android::content::Intent, addFlags, jint)
    /** Returns the data of this intent. */
    JNI_METHOD(android::net::Uri, getData)
    /** Get the intent action. */
    JNI_METHOD(java::lang::String, getAction)
    /** Sets the action of the intent. */
    JNI_METHOD(android::content::Intent, setAction, java::lang::String)
    /** Set the data URL of this intent. */
    JNI_METHOD(android::content::Intent, setData, android::net::Uri)

    /** Returns the mimetype of this intent. */
    JNI_METHOD(java::lang::String, getType)
    /** Set the mime type for this intent. */
    JNI_METHOD(android::content::Intent, setType, java::lang::String)

    /** Read extra intent data. */
    QString getStringExtra(const QJniObject &name) const;
    QStringList getStringArrayExtra(const QJniObject &name) const;
    /** Add extra intent data of type @tparam T. */
    template<typename T>
    inline void putExtra(const QJniObject &name, const QJniObject &value)
    {
        jniHandle().callObjectMethod("putExtra", Jni::signature<android::content::Intent(java::lang::String, T)>(), name.object(), value.object());
    }

    /** Implicit conversion to an QJniObject. */
    operator QJniObject() const;

    /** Action constant for create document intents. */
    JNI_CONSTANT(java::lang::String, ACTION_CREATE_DOCUMENT)
    /** Main activity entry point. */
    JNI_CONSTANT(java::lang::String, ACTION_MAIN)
    /** Action constant for open document intents. */
    JNI_CONSTANT(java::lang::String, ACTION_OPEN_DOCUMENT)
    /** Action constant for viewing intents. */
    JNI_CONSTANT(java::lang::String, ACTION_VIEW)
    /** Share data. */
    JNI_CONSTANT(java::lang::String, ACTION_SEND)
    /** Share multiple data items. */
    JNI_CONSTANT(java::lang::String, ACTION_SEND_MULTIPLE)

    /** Category constant for openable content. */
    JNI_CONSTANT(java::lang::String, CATEGORY_OPENABLE)

    JNI_CONSTANT(java::lang::String, EXTRA_EMAIL)
    JNI_CONSTANT(java::lang::String, EXTRA_STREAM)
    JNI_CONSTANT(java::lang::String, EXTRA_SUBJECT)
    JNI_CONSTANT(java::lang::String, EXTRA_TEXT)

    /** Flag for granting read URI permissions on content providers. */
    JNI_CONSTANT(jint, FLAG_GRANT_READ_URI_PERMISSION)
    /** Flag for granting write URI permissions on content providers. */
    JNI_CONSTANT(jint, FLAG_GRANT_WRITE_URI_PERMISSION)

private:
    template<typename T>
    QJniObject getObjectExtra(const char *methodName, const QJniObject &name) const;
};

}

#endif // KANDROIDEXTRAS_INTENT_H
