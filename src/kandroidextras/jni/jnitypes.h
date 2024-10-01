/*
    SPDX-FileCopyrightText: 2019 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNITYPES_H
#define KANDROIDEXTRAS_JNITYPES_H

#include "jnipp.h"

/** C++/Android integration utilities built on top of @c QJniObject. */
namespace KAndroidExtras
{

///@cond internal

// preprocessor "iteration" for regular classes
#define JNI_TYPE_1(name, type, ...)                                                                                                                            \
    struct type {                                                                                                                                              \
        static constexpr const char *jniName()                                                                                                                 \
        {                                                                                                                                                      \
            return name #type;                                                                                                                                 \
        }                                                                                                                                                      \
    };
#define JNI_TYPE_2(name, type, ...)                                                                                                                            \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_TYPE_1(name #type "/", __VA_ARGS__)                                                                                                                    \
    }
#define JNI_TYPE_3(name, type, ...)                                                                                                                            \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_TYPE_2(name #type "/", __VA_ARGS__)                                                                                                                    \
    }
#define JNI_TYPE_4(name, type, ...)                                                                                                                            \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_TYPE_3(name #type "/", __VA_ARGS__)                                                                                                                    \
    }
#define JNI_TYPE_5(name, type, ...)                                                                                                                            \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_TYPE_4(name #type "/", __VA_ARGS__)                                                                                                                    \
    }
#define JNI_TYPE_6(name, type, ...)                                                                                                                            \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_TYPE_5(name #type "/", __VA_ARGS__)                                                                                                                    \
    }
#define JNI_TYPE_7(name, type, ...)                                                                                                                            \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_TYPE_6(#type "/", __VA_ARGS__)                                                                                                                         \
    }
#define JNI_TYPE_(N, name, ...) JNI_PP_CONCAT(JNI_TYPE_, N)(name, __VA_ARGS__)

// preprocessor "iteration" for nested classes
#define JNI_NESTED_TYPE_2(name, type, nested_type, ...)                                                                                                        \
    struct type##_##nested_type {                                                                                                                              \
        static constexpr const char *jniName()                                                                                                                 \
        {                                                                                                                                                      \
            return name #type "$" #nested_type;                                                                                                                \
        }                                                                                                                                                      \
    };
#define JNI_NESTED_TYPE_3(name, type, ...)                                                                                                                     \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_NESTED_TYPE_2(name #type "/", __VA_ARGS__)                                                                                                             \
    }
#define JNI_NESTED_TYPE_4(name, type, ...)                                                                                                                     \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_NESTED_TYPE_3(name #type "/", __VA_ARGS__)                                                                                                             \
    }
#define JNI_NESTED_TYPE_5(name, type, ...)                                                                                                                     \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_NESTED_TYPE_4(name #type "/", __VA_ARGS__)                                                                                                             \
    }
#define JNI_NESTED_TYPE_6(name, type, ...)                                                                                                                     \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_NESTED_TYPE_5(name #type "/", __VA_ARGS__)                                                                                                             \
    }
#define JNI_NESTED_TYPE_7(name, type, ...)                                                                                                                     \
    namespace type                                                                                                                                             \
    {                                                                                                                                                          \
    JNI_NESTED_TYPE_6(#type "/", __VA_ARGS__)                                                                                                                  \
    }
#define JNI_NESTED_TYPE_(N, name, ...) JNI_PP_CONCAT(JNI_NESTED_TYPE_, N)(name, __VA_ARGS__)

///@endcond

/** Macro to define Java types with their corresponding JNI signature strings. */
#define JNI_TYPE(...) JNI_TYPE_(JNI_PP_NARG(__VA_ARGS__), "", __VA_ARGS__)

/** Macro to define a nested Java class with its corresponding JNI signature string. */
#define JNI_NESTED_TYPE(...) JNI_NESTED_TYPE_(JNI_PP_NARG(__VA_ARGS__), "", __VA_ARGS__)

/** Functions for interfacing with the Java Native Interface (JNI). */
namespace Jni
{
/** Returns the JNI type name of the given template argument. */
template<typename T>
inline constexpr const char *typeName()
{
    return T::jniName();
}
}

}

#endif // KANDROIDEXTRAS_JNITYPES_H
