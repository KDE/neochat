/*
    SPDX-FileCopyrightText: 2021-2022 Volker Krause <vkrause@kde.org>
    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIARGUMENTVALUE_H
#define KANDROIDEXTRAS_JNIARGUMENTVALUE_H

#include "jniobject.h"
#include "jnitypetraits.h"

namespace KAndroidExtras
{
namespace Jni
{
template<typename T>
class Array;
}

///@cond internal
namespace Internal
{
/** Call argument wrapper. */
template<typename T, typename = std::void_t<>>
struct argument {
    static_assert(!is_invalid_primitive_type<T>::value, "Using an incompatible primitive type!");
    typedef std::conditional_t<Jni::is_primitive_type<T>::value, T, const Jni::Object<T> &> type;
    static inline constexpr auto toCallArgument(type value)
    {
        if constexpr (Jni::is_primitive_type<T>::value) {
            return primitive_value<T>::toJni(value);
        } else {
            return value.jniHandle().object();
        }
    }
};
template<typename T>
struct argument<T, std::void_t<typename T::_jni_ThisType>> {
    typedef const T &type;
    static inline auto toCallArgument(const T &value)
    {
        return Jni::handle(value).object();
    }
};
template<typename T>
struct argument<Jni::Array<T>> {
    typedef const Jni::Array<T> &type;
    static inline auto toCallArgument(const Jni::Array<T> &value)
    {
        return value.jniHandle().object();
    }
};
}
///@endcond
}

#endif
