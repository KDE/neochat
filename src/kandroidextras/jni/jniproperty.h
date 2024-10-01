/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIPROPERTIES_H
#define KANDROIDEXTRAS_JNIPROPERTIES_H

#include "jniargument.h"
#include "jniobject.h"
#include "jnisignature.h"
#include "jnitypes.h"
#include "jnitypetraits.h"

#include <type_traits>

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

/** Wrapper for static properties. */
template<typename PropType, typename ClassType, typename NameHolder, bool PrimitiveType>
struct StaticProperty {
};
template<typename PropType, typename ClassType, typename NameHolder>
struct StaticProperty<PropType, ClassType, NameHolder, false> {
    static_assert(!is_invalid_primitive_type<PropType>::value, "Using an incompatible primitive type!");
    inline QJniObject get() const
    {
        return QJniObject::getStaticObjectField(Jni::typeName<ClassType>(), Jni::typeName<NameHolder>(), Jni::signature<PropType>());
    }
    inline operator QJniObject() const
    {
        return get();
    }
    inline operator typename Jni::converter<PropType>::type() const
    {
        return Jni::converter<PropType>::convert(get());
    }
    template<typename RetT = PropType, typename = std::enable_if_t<Jni::is_generic_wrapper<PropType>::value, RetT>>
    inline operator Jni::Object<PropType>() const
    {
        return Jni::Object<PropType>(get());
    }
};

template<typename PropType, typename ClassType, typename NameHolder>
struct StaticProperty<PropType, ClassType, NameHolder, true> {
    inline operator auto() const
    {
        return primitive_value<PropType>::fromJni(
            QJniObject::getStaticField<typename primitive_value<PropType>::JniType>(Jni::typeName<ClassType>(), Jni::typeName<NameHolder>()));
    }
};

/** Shared code for non-static property wrappers. */
template<typename ClassType, typename OffsetHolder>
class PropertyBase
{
protected:
    inline QJniObject handle() const
    {
        const auto owner = reinterpret_cast<const ClassType *>(reinterpret_cast<const char *>(this) - OffsetHolder::offset());
        return owner->jniHandle();
    }
};

/** Wrapper for non-static properties. */
template<typename PropType, typename ClassType, typename NameHolder, typename OffsetHolder, bool PrimitiveType>
struct Property {
};
template<typename PropType, typename ClassType, typename NameHolder, typename OffsetHolder>
class Property<PropType, ClassType, NameHolder, OffsetHolder, false> : public PropertyBase<ClassType, OffsetHolder>
{
private:
    struct _jni_NoType {
    };
    static_assert(!is_invalid_primitive_type<PropType>::value, "Using an incompatible primitive type!");

public:
    inline QJniObject get() const
    {
        return this->handle().getObjectField(Jni::typeName<NameHolder>(), Jni::signature<PropType>());
    }
    inline operator QJniObject() const
    {
        return get();
    }
    inline operator typename Jni::converter<PropType>::type() const
    {
        return Jni::converter<PropType>::convert(get());
    }
    template<typename RetT = PropType, typename = std::enable_if_t<Jni::is_array<PropType>::value, RetT>>
    inline operator PropType() const
    {
        return PropType(get());
    }
    template<typename RetT = PropType, typename = std::enable_if_t<Jni::is_generic_wrapper<PropType>::value, RetT>>
    inline operator Jni::Object<PropType>() const
    {
        return Jni::Object<PropType>(get());
    }

    inline Property &operator=(typename Internal::argument<PropType>::type value)
    {
        this->handle().setField(Jni::typeName<NameHolder>(), Jni::signature<PropType>(), Internal::argument<PropType>::toCallArgument(value));
        return *this;
    }
    inline Property &operator=(const QJniObject &value)
    {
        this->handle().setField(Jni::typeName<NameHolder>(), Jni::signature<PropType>(), value.object());
        return *this;
    }
    inline Property &operator=(const typename std::conditional<std::is_same_v<typename Jni::converter<PropType>::type, void>,
                                                               _jni_NoType,
                                                               typename Jni::converter<PropType>::type>::type &value)
    {
        this->handle().setField(Jni::typeName<NameHolder>(), Jni::signature<PropType>(), Jni::reverse_converter<PropType>::type::convert(value).object());
        return *this;
    }

    // special case for string comparison, which is often done against different types and thus the implicit conversion operator
    // isn't going to be enough
    template<typename CmpT, typename = std::enable_if_t<std::is_same_v<PropType, java::lang::String>, CmpT>>
    inline bool operator==(const CmpT &other) const
    {
        return QString(*this) == other;
    }
};

template<typename PropType, typename ClassType, typename NameHolder, typename OffsetHolder>
class Property<PropType, ClassType, NameHolder, OffsetHolder, true> : public PropertyBase<ClassType, OffsetHolder>
{
public:
    inline operator auto() const
    {
        return primitive_value<PropType>::fromJni(this->handle().template getField<typename primitive_value<PropType>::JniType>(Jni::typeName<NameHolder>()));
    }
    inline Property &operator=(PropType value)
    {
        this->handle().setField(Jni::typeName<NameHolder>(), primitive_value<PropType>::toJni(value));
        return *this;
    }
};

// TODO KF6: can be replaced by QT_WARNING_DISABLE_INVALID_OFFSETOF
#if defined(Q_CC_CLANG)
#define JNI_WARNING_DISABLE_INVALID_OFFSETOF QT_WARNING_DISABLE_CLANG("-Winvalid-offsetof")
#elif defined(Q_CC_GNU)
#define JNI_WARNING_DISABLE_INVALID_OFFSETOF QT_WARNING_DISABLE_GCC("-Winvalid-offsetof")
#else
#define JNI_WARNING_DISABLE_INVALID_OFFSETOF
#endif

/** @endcond */
}

/**
 * Wrap a static final property.
 * This will add a public static member named @p name to the current class. This member defines an
 * implicit conversion operator which will trigger the corresponding a JNI read operation.
 * Can only be placed in classes with a @c JNI_OBJECT.
 *
 * @note Make sure to access this member with a specific type, assigning to an @c auto variable will
 * copy the wrapper type, not read the property value.
 *
 * @param type The data type of the property.
 * @param name The name of the property.
 */
#define JNI_CONSTANT(type, name)                                                                                                                               \
private:                                                                                                                                                       \
    struct _jni_##name##__NameHolder {                                                                                                                         \
        static constexpr const char *jniName()                                                                                                                 \
        {                                                                                                                                                      \
            return "" #name;                                                                                                                                   \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
                                                                                                                                                               \
public:                                                                                                                                                        \
    static inline const KAndroidExtras::Internal::StaticProperty<type, _jni_ThisType, _jni_##name##__NameHolder, Jni::is_primitive_type<type>::value> name;

/**
 * Wrap a member property.
 * This will add a public zero-size member named @p name to the current class. This member defines an
 * implicit conversion operator which will trigger the corresponding a JNI read operation, as well
 * as an overloaded assignment operator for the corresponding write operation.
 * Can only be placed in classes with a @c JNI_OBJECT.
 *
 * @note Make sure to access this member with a specific type, assigning to an @c auto variable will
 * copy the wrapper type, not read the property value.
 *
 * @param type The data type of the property.
 * @param name The name of the property.
 */
#define JNI_PROPERTY(type, name)                                                                                                                               \
private:                                                                                                                                                       \
    struct _jni_##name##__NameHolder {                                                                                                                         \
        static constexpr const char *jniName()                                                                                                                 \
        {                                                                                                                                                      \
            return "" #name;                                                                                                                                   \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
    struct _jni_##name##__OffsetHolder {                                                                                                                       \
        static constexpr std::size_t offset()                                                                                                                  \
        {                                                                                                                                                      \
            QT_WARNING_PUSH JNI_WARNING_DISABLE_INVALID_OFFSETOF return offsetof(_jni_ThisType, name);                                                         \
            QT_WARNING_POP                                                                                                                                     \
        }                                                                                                                                                      \
    };                                                                                                                                                         \
    friend class KAndroidExtras::Internal::PropertyBase<_jni_ThisType, _jni_##name##__OffsetHolder>;                                                           \
                                                                                                                                                               \
public:                                                                                                                                                        \
    [[no_unique_address]] KAndroidExtras::Internal::                                                                                                           \
        Property<type, _jni_ThisType, _jni_##name##__NameHolder, _jni_##name##__OffsetHolder, KAndroidExtras::Jni::is_primitive_type<type>::value> name;
}

#endif // KANDROIDEXTRAS_JNIPROPERTIES_H
