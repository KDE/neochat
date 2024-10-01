/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNITYPETRAITS_H
#define KANDROIDEXTRAS_JNITYPETRAITS_H

#include "jniprimitivetypes.h"
#include "jnitypes.h"

#include <QJniObject>

namespace KAndroidExtras
{

namespace Jni
{

template<typename T>
class Array;

/** Type conversion trait, see @c JNI_DECLARE_CONVERTER. */
template<typename T>
struct converter {
    typedef void type;
};

template<typename T>
struct reverse_converter {
    typedef converter<typename converter<T>::type> type;
};

/** Type trait for checking whether @tparam T is a JNI array type. */
template<typename T>
struct is_array : std::false_type {
};
template<typename T>
struct is_array<Array<T>> : std::true_type {
};

/** Type trais for checking whether @tparam T is a JNI object wrapper type. */
template<typename T, typename = std::void_t<>>
struct is_object_wrapper : std::false_type {
};
template<typename T>
struct is_object_wrapper<T, std::void_t<typename T::_jni_ThisType>> : std::true_type {
};

/** Type trais for checking whether @tparam T is needs the generic JNI object wrapper (Jni::Object). */
template<typename T>
struct is_generic_wrapper
    : std::conditional_t<!is_primitive_type<T>::value && !is_array<T>::value && !is_object_wrapper<T>::value, std::true_type, std::false_type> {
};

}

/**
 * Declare a JNI type to be convertible to a native type.
 * @param JniType A type declared with @p JNI_TYPE.
 * @param NativeType A C++ type @p JniType can be converted to/from.
 * @param FromJniFn Code converting a @c QJniObject @c value to @p NativeType.
 * @param ToJniCode converting a @p NativeType @c value to a QJniObject.
 */
#define JNI_DECLARE_CONVERTER(JniType, NativeType, FromJniFn, ToJniFn)                                                                                         \
    namespace Jni                                                                                                                                              \
    {                                                                                                                                                          \
    template<>                                                                                                                                                 \
    struct converter<NativeType> {                                                                                                                             \
        typedef JniType type;                                                                                                                                  \
        static inline QJniObject convert(const NativeType &value)                                                                                              \
        {                                                                                                                                                      \
            return (ToJniFn);                                                                                                                                  \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
    template<>                                                                                                                                                 \
    struct converter<JniType> {                                                                                                                                \
        typedef NativeType type;                                                                                                                               \
        static inline NativeType convert(const QJniObject &value)                                                                                              \
        {                                                                                                                                                      \
            return (FromJniFn);                                                                                                                                \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
    }

}

#endif
