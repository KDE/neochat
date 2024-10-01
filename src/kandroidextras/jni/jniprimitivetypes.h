/*
    SPDX-FileCopyrightText: 2020-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIPRIMITIVETYPES_H
#define KANDROIDEXTRAS_JNIPRIMITIVETYPES_H

#include "jni.h"

#include <type_traits>

namespace KAndroidExtras
{

namespace Jni
{

/** Type trait to check whether @tparam T is a primitive JNI type or an object type. */
template<typename T>
struct is_primitive_type : std::false_type {
};
template<>
struct is_primitive_type<jboolean> : std::true_type {
};
template<>
struct is_primitive_type<jbyte> : std::true_type {
};
template<>
struct is_primitive_type<jchar> : std::true_type {
};
template<>
struct is_primitive_type<jshort> : std::true_type {
};
template<>
struct is_primitive_type<jint> : std::true_type {
};
template<>
struct is_primitive_type<jlong> : std::true_type {
};
template<>
struct is_primitive_type<jfloat> : std::true_type {
};
template<>
struct is_primitive_type<jdouble> : std::true_type {
};
template<>
struct is_primitive_type<void> : std::true_type {
};
// special case for bool <-> jboolean conversion
template<>
struct is_primitive_type<bool> : std::true_type {
};

}

namespace Internal
{

/** Utility trait to check for basic C++ types that are not JNI primitive types. */
template<typename T>
struct is_invalid_primitive_type : std::false_type {
};
template<>
struct is_invalid_primitive_type<uint8_t> : std::true_type {
};
template<>
struct is_invalid_primitive_type<char> : std::true_type {
};
template<>
struct is_invalid_primitive_type<uint32_t> : std::true_type {
};
template<>
struct is_invalid_primitive_type<uint64_t> : std::true_type {
};

// C++ primitive to JNI primitive conversion
template<typename T>
struct primitive_value {
    static_assert(!is_invalid_primitive_type<T>::value, "Using an incompatible primitive type!");
    typedef T JniType;
    typedef T NativeType;
    static constexpr inline T toJni(T value)
    {
        return value;
    }
    static constexpr inline T fromJni(T value)
    {
        return value;
    }
};
template<>
struct primitive_value<void> {
};
template<>
struct primitive_value<jboolean> {
    typedef jboolean JniType;
    typedef bool NativeType;
    static constexpr inline jboolean toJni(bool value)
    {
        return value ? 1 : 0;
    }
    static constexpr inline bool fromJni(jboolean value)
    {
        return value != 0;
    }
};
template<>
struct primitive_value<bool> {
    typedef jboolean JniType;
    typedef bool NativeType;
    static constexpr inline jboolean toJni(bool value)
    {
        return value ? 1 : 0;
    }
    static constexpr inline bool fromJni(jboolean value)
    {
        return value != 0;
    }
};

}
}

#endif
