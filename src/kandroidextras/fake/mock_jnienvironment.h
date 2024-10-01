/*
    SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_MOCK_JNIENVIRONMENT_H
#define KANDROIDEXTRAS_MOCK_JNIENVIRONMENT_H

#include "jni.h"

namespace KAndroidExtras
{
class MockJniEnvironment
{
public:
    inline jclass findClass(const char *)
    {
        return nullptr;
    }
    inline JNIEnv *operator->()
    {
        return &m_env;
    }

protected:
    mutable JNIEnv m_env;
};
}

#endif
