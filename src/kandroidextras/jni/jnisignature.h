/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNISIGNATURE_H
#define KANDROIDEXTRAS_JNISIGNATURE_H

#include "jnitypes.h"

#include "jni.h"
#include <cstdint>
#include <utility>

namespace KAndroidExtras
{

namespace Jni
{
template<typename T>
class Array;
}

/** @cond internal */
namespace Internal
{

/** Compile-time concat-able string. */
template<char... String>
struct StaticString {
    inline operator const char *() const
    {
        static const char data[] = {String..., 0};
        return data;
    }
};

/** Compile-time strlen. */
constexpr inline int static_strlen(const char *str)
{
    return str[0] == '\0' ? 0 : static_strlen(str + 1) + 1;
}

/** Compile-time concat for two StaticString. */
template<char... String1, char... String2>
constexpr inline auto static_concat(const StaticString<String1...> &, const StaticString<String2...> &)
{
    return StaticString<String1..., String2...>();
}

/** Compile-time concept for N StaticString. */
template<typename String1, typename String2, class... Strings>
constexpr inline auto static_concat(const String1 &str1, const String2 &str2, const Strings &...strs)
{
    return static_concat(static_concat(str1, str2), strs...);
}

/** Conversion from const char* literals to StaticString. */
template<typename, typename>
struct staticStringFromJniType;
template<typename T, std::size_t... Indexes>
struct staticStringFromJniType<T, std::index_sequence<Indexes...>> {
    typedef StaticString<Jni::typeName<T>()[Indexes]...> value;
};

/** Meta function for assembling JNI signatures. */
template<typename T>
struct JniSignature {
    constexpr inline auto operator()() const
    {
        using namespace Internal;
        return static_concat(StaticString<'L'>(),
                             typename staticStringFromJniType<T, std::make_index_sequence<static_strlen(Jni::typeName<T>())>>::value(),
                             StaticString<';'>());
    }
};

template<>
struct JniSignature<jboolean> {
    constexpr inline auto operator()() const
    {
        return StaticString<'Z'>();
    }
};
template<>
struct JniSignature<jbyte> {
    constexpr inline auto operator()() const
    {
        return StaticString<'B'>();
    }
};
template<>
struct JniSignature<jchar> {
    constexpr inline auto operator()() const
    {
        return StaticString<'C'>();
    }
};
template<>
struct JniSignature<jshort> {
    constexpr inline auto operator()() const
    {
        return StaticString<'S'>();
    }
};
template<>
struct JniSignature<jint> {
    constexpr inline auto operator()() const
    {
        return StaticString<'I'>();
    }
};
template<>
struct JniSignature<jlong> {
    constexpr inline auto operator()() const
    {
        return StaticString<'J'>();
    }
};
template<>
struct JniSignature<jfloat> {
    constexpr inline auto operator()() const
    {
        return StaticString<'F'>();
    }
};
template<>
struct JniSignature<jdouble> {
    constexpr inline auto operator()() const
    {
        return StaticString<'D'>();
    }
};
template<>
struct JniSignature<void> {
    constexpr inline auto operator()() const
    {
        return StaticString<'V'>();
    }
};
// special case due to jboolean != bool
template<>
struct JniSignature<bool> {
    constexpr inline auto operator()() const
    {
        return StaticString<'Z'>();
    }
};

template<typename T>
struct JniSignature<T *> {
    constexpr inline auto operator()() const
    {
        using namespace Internal;
        return static_concat(StaticString<'['>(), JniSignature<T>()());
    }
};

template<typename T>
struct JniSignature<Jni::Array<T>> {
    constexpr inline auto operator()() const
    {
        using namespace Internal;
        return static_concat(StaticString<'['>(), JniSignature<T>()());
    }
};

template<typename RetT, typename... Args>
struct JniSignature<RetT(Args...)> {
    constexpr inline auto operator()() const
    {
        using namespace Internal;
        return static_concat(StaticString<'('>(), JniSignature<Args>()()..., StaticString<')'>(), JniSignature<RetT>()());
    }
};
}
/** @endcond */

/** Helper methods to deal with JNI. */
namespace Jni
{
/** Returns the JNI signature string for the template argument types. */
template<typename... Args>
constexpr __attribute__((__unused__)) Internal::JniSignature<Args...> signature = {};
}

}

#endif // KANDROIDEXTRAS_JNISIGNATURE_H
