/*
    SPDX-FileCopyrightText: 2020 Volker Krause <vkrause@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KANDROIDEXTRAS_JNIARRAY_H
#define KANDROIDEXTRAS_JNIARRAY_H

#include "jniargument.h"
#include "jnireturnvalue.h"
#include "jnitypetraits.h"

#include <QJniEnvironment>

namespace KAndroidExtras
{

///@cond internal
namespace Internal
{

/** Primitive type array type traits. */
template<typename T>
struct array_trait {
    typedef jobjectArray type;
};

#define MAKE_ARRAY_TRAIT(base_type, type_name)                                                                                                                 \
    template<>                                                                                                                                                 \
    struct array_trait<base_type> {                                                                                                                            \
        typedef base_type##Array type;                                                                                                                         \
        static inline type newArray(QJniEnvironment &env, jsize size)                                                                                          \
        {                                                                                                                                                      \
            return env->New##type_name##Array(size);                                                                                                           \
        }                                                                                                                                                      \
        static inline base_type *getArrayElements(QJniEnvironment &env, type array, jboolean *isCopy)                                                          \
        {                                                                                                                                                      \
            return env->Get##type_name##ArrayElements(array, isCopy);                                                                                          \
        }                                                                                                                                                      \
        static inline void releaseArrayElements(QJniEnvironment &env, type array, base_type *data, jint mode)                                                  \
        {                                                                                                                                                      \
            return env->Release##type_name##ArrayElements(array, data, mode);                                                                                  \
        }                                                                                                                                                      \
        static inline void setArrayRegion(QJniEnvironment &env, type array, jsize start, jsize length, const base_type *data)                                  \
        {                                                                                                                                                      \
            env->Set##type_name##ArrayRegion(array, start, length, data);                                                                                      \
        }                                                                                                                                                      \
    };

MAKE_ARRAY_TRAIT(jboolean, Boolean)
MAKE_ARRAY_TRAIT(jbyte, Byte)
MAKE_ARRAY_TRAIT(jchar, Char)
MAKE_ARRAY_TRAIT(jshort, Short)
MAKE_ARRAY_TRAIT(jint, Int)
MAKE_ARRAY_TRAIT(jlong, Long)
MAKE_ARRAY_TRAIT(jfloat, Float)
MAKE_ARRAY_TRAIT(jdouble, Double)

#undef MAKE_ARRAY_TRAIT

/** Meta function for retrieving a JNI array .*/
template<typename Container, typename Value, bool is_primitive>
struct FromArray {
};

template<typename Container>
struct FromArray<Container, QJniObject, false> {
    inline auto operator()(const QJniObject &array) const
    {
        if (!array.isValid()) {
            return Container{};
        }
        const auto a = static_cast<jobjectArray>(array.object());
        QJniEnvironment env;
        const auto size = env->GetArrayLength(a);
        Container r;
        r.reserve(size);
        for (auto i = 0; i < size; ++i) {
            r.push_back(QJniObject::fromLocalRef(env->GetObjectArrayElement(a, i)));
        }
        return r;
    }
};

template<typename Container, typename Value>
struct FromArray<Container, Value, false> {
    inline auto operator()(const QJniObject &array) const
    {
        if (!array.isValid()) {
            return Container{};
        }
        const auto a = static_cast<jobjectArray>(array.object());
        QJniEnvironment env;
        const auto size = env->GetArrayLength(a);
        Container r;
        r.reserve(size);
        for (auto i = 0; i < size; ++i) {
            r.push_back(Jni::reverse_converter<Value>::type::convert(QJniObject::fromLocalRef(env->GetObjectArrayElement(a, i))));
        }
        return r;
    }
};

// specializations for primitive types
template<typename Container, typename Value>
struct FromArray<Container, Value, true> {
    typedef array_trait<Value> _t;
    inline auto operator()(const QJniObject &array) const
    {
        if (!array.isValid()) {
            return Container{};
        }

        const auto a = static_cast<typename _t::type>(array.object());
        QJniEnvironment env;
        const auto size = env->GetArrayLength(a);
        Container r;
        r.reserve(size);

        auto data = _t::getArrayElements(env, a, nullptr);
        std::copy(data, data + size, std::back_inserter(r));
        _t::releaseArrayElements(env, a, data, JNI_ABORT);

        return r;
    }
};

// array wrapper, common base for primitive and non-primitive types
template<typename T>
class ArrayImplBase
{
public:
    typedef T value_type;
    typedef jsize size_type;
    typedef jsize difference_type;

    ArrayImplBase() = default;
    inline ArrayImplBase(const QJniObject &array)
        : m_array(array)
    {
    }
    ArrayImplBase(const ArrayImplBase &) = default;
    ArrayImplBase(ArrayImplBase &&) = default;

    inline size_type size() const
    {
        if (!m_array.isValid()) {
            return 0;
        }
        const auto a = static_cast<typename _t::type>(m_array.object());
        QJniEnvironment env;
        return env->GetArrayLength(a);
    }

    inline operator QJniObject() const
    {
        return m_array;
    }
    inline QJniObject jniHandle() const
    {
        return m_array;
    }

protected:
    typedef array_trait<T> _t;

    typename _t::type handle() const
    {
        return static_cast<typename _t::type>(m_array.object());
    }

    QJniObject m_array;
};

template<typename T, bool is_primitive>
class ArrayImpl
{
};

// array wrapper for primitive types
template<typename T>
class ArrayImpl<T, true> : public ArrayImplBase<T>
{
    static_assert(!Internal::is_invalid_primitive_type<T>::value, "Using an incompatible primitive type!");

public:
    inline ArrayImpl(const QJniObject &array)
        : ArrayImplBase<T>(array)
    {
        // ### do this on demand?
        getArrayElements();
    }

    /** Create a new array with @p size elements. */
    inline explicit ArrayImpl(jsize size)
    {
        QJniEnvironment env;
        ArrayImplBase<T>::m_array = QJniObject::fromLocalRef(ArrayImplBase<T>::_t::newArray(env, size));
        getArrayElements();
    }

    ArrayImpl() = default;
    ArrayImpl(const ArrayImpl &) = delete; // ### ref count m_data and allow copying?
    ArrayImpl(ArrayImpl &&) = default;
    ~ArrayImpl()
    {
        QJniEnvironment env;
        ArrayImplBase<T>::_t::releaseArrayElements(env, this->handle(), m_data, JNI_ABORT);
    }

    T operator[](jsize index) const
    {
        return m_data[index];
    }

    T *begin() const
    {
        return m_data;
    }
    T *end() const
    {
        return m_data + ArrayImplBase<T>::size();
    }

private:
    inline void getArrayElements()
    {
        if (!ArrayImplBase<T>::m_array.isValid()) {
            return;
        }
        QJniEnvironment env;
        m_data = ArrayImplBase<T>::_t::getArrayElements(env, this->handle(), nullptr);
    }

    T *m_data = nullptr;
};

// array wrapper for non-primitive types
template<typename T>
class ArrayImpl<T, false> : public ArrayImplBase<T>
{
public:
    using ArrayImplBase<T>::ArrayImplBase;

    /** Create a new array with @p size elements initialized with @p value. */
    explicit inline ArrayImpl(jsize size, typename Internal::argument<T>::type value)
    {
        QJniEnvironment env;
        auto clazz = env.findClass(Jni::typeName<T>());
        ArrayImplBase<T>::m_array = QJniObject::fromLocalRef(env->NewObjectArray(size, clazz, Internal::argument<T>::toCallArgument(value)));
    }

    /** Create a new array with @p size null elements. */
    explicit inline ArrayImpl(jsize size, std::nullptr_t = nullptr)
    {
        QJniEnvironment env;
        auto clazz = env.findClass(Jni::typeName<T>());
        ArrayImplBase<T>::m_array = QJniObject::fromLocalRef(env->NewObjectArray(size, clazz, nullptr));
    }

    ArrayImpl() = default;
    ArrayImpl(const ArrayImpl &) = default;
    ArrayImpl(ArrayImpl &&) = default;

    auto operator[](jsize index) const
    {
        QJniEnvironment env;
        return Internal::return_wrapper<T>::toReturnValue(QJniObject::fromLocalRef(env->GetObjectArrayElement(this->handle(), index)));
    }

    class ref
    {
    public:
        inline operator auto()
        {
            QJniEnvironment env;
            return Internal::return_wrapper<T>::toReturnValue(QJniObject::fromLocalRef(env->GetObjectArrayElement(c.handle(), index)));
        }
        inline ref &operator=(typename Internal::argument<T>::type v)
        {
            QJniEnvironment env;
            env->SetObjectArrayElement(c.handle(), index, Internal::argument<T>::toCallArgument(v));
            return *this;
        }

    private:
        ArrayImpl<T, false> &c;
        jsize index;

        friend class ArrayImpl<T, false>;
        inline ref(jsize _i, ArrayImpl<T, false> &_c)
            : c(_c)
            , index(_i)
        {
        }
    };
    ref operator[](jsize index)
    {
        return ref(index, *this);
    }

    class const_iterator
    {
        const ArrayImpl<T, false> &c;
        jsize i = 0;

    public:
        typedef jsize difference_type;
        typedef T value_type;
        typedef T &reference;
        typedef std::random_access_iterator_tag iterator_category;
        typedef T *pointer;

        const_iterator(const ArrayImpl<T, false> &_c, jsize _i)
            : c(_c)
            , i(_i)
        {
        }

        difference_type operator-(const_iterator other) const
        {
            return i - other.i;
        }

        const_iterator &operator++()
        {
            ++i;
            return *this;
        }
        const_iterator operator++(int)
        {
            return const_iterator(c, i++);
        }

        bool operator==(const_iterator other) const
        {
            return i == other.i;
        }
        bool operator!=(const_iterator other) const
        {
            return i != other.i;
        }

        auto operator*() const
        {
            return c[i];
        }
    };

    const_iterator begin() const
    {
        return const_iterator(*this, 0);
    }
    const_iterator end() const
    {
        return const_iterator(*this, ArrayImplBase<T>::size());
    }
};

}
///@endcond

namespace Jni
{

/** Convert a JNI array to a C++ container.
 *  Container value types can be any of
 *  - QJniObject
 *  - a primitive JNI type
 *  - a type with a conversion defined with @c JNI_DECLARE_CONVERTER
 */
template<typename Container>
constexpr
    __attribute__((__unused__)) Internal::FromArray<Container, typename Container::value_type, Jni::is_primitive_type<typename Container::value_type>::value>
        fromArray = {};

/** Container-like wrapper for JNI arrays. */
template<typename T>
class Array : public Internal::ArrayImpl<T, Jni::is_primitive_type<T>::value>
{
public:
    using Internal::ArrayImpl<T, Jni::is_primitive_type<T>::value>::ArrayImpl;
    template<typename Container>
    inline operator Container() const
    {
        // ### should this be re-implemented in terms of Jni::Array API rather than direct JNI access?
        return Jni::fromArray<Container>(this->m_array);
    }
};
}

}

#endif
