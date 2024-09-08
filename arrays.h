#pragma once

#include "jni_types.h"

namespace jnipp::java::array_extractors {

namespace detail {

template<return_type T>
inline auto get_element(::jarray, ::jsize)
{
}

template<>
inline auto get_element<return_type::object_>(::jarray instance, ::jsize i)
{
    return GetJNI()->GetObjectArrayElement(
        reinterpret_cast<jobjectArray>(instance), i);
}

#define DEFINE_GET_ELEMENT(JAVA_TYPE, JAVA_NAME, RETURN_TYPE)          \
    template<>                                                         \
    inline auto get_element<RETURN_TYPE>(::jarray instance, ::jsize i) \
    {                                                                  \
        JAVA_TYPE out;                                                 \
        GetJNI()->Get##JAVA_NAME##ArrayRegion(                         \
            reinterpret_cast<JAVA_TYPE##Array>(instance), i, 1, &out); \
        return out;                                                    \
    }

DEFINE_GET_ELEMENT(::jboolean, Boolean, return_type::bool_)
DEFINE_GET_ELEMENT(::jbyte, Byte, return_type::byte_)
DEFINE_GET_ELEMENT(::jchar, Char, return_type::char_)
DEFINE_GET_ELEMENT(::jshort, Short, return_type::short_)
DEFINE_GET_ELEMENT(::jint, Int, return_type::int_)
DEFINE_GET_ELEMENT(::jlong, Long, return_type::long_)
DEFINE_GET_ELEMENT(::jfloat, Float, return_type::float_)
DEFINE_GET_ELEMENT(::jdouble, Double, return_type::double_)

#undef DEFINE_GET_ELEMENT

} // namespace detail

template<return_type T>
struct extract_type
{
    extract_type(java::array array)
        : ref(array)
    {
    }

    jlong length() const
    {
        return ref.length();
    }

    auto operator[](jsize index)
    {
        if(index >= length())
            throw std::out_of_range(
                std::to_string(index) + " >= " + std::to_string(length()));

        if constexpr(T == return_type::object_)
            return java::object{
                {},
                GetJNI()->GetObjectArrayElement(
                    reinterpret_cast<jobjectArray>(ref.instance), index)};
        else if constexpr(T != return_type::object_)
            return detail::get_element<T>(ref.instance, index);
        else
            return java::value();
    }

    java::array ref;
};

template<return_type T>
struct container
{
    container(java::array arrayObject)
        : m_extractor(arrayObject)
        , m_end(m_extractor.length())
    {
    }

    struct iterator
    {
        iterator(container<T>& container, jsize idx)
            : m_ref(container)
            , m_idx(idx)
        {
        }

        iterator(container<T>& container)
            : m_ref(container)
            , m_idx(m_ref.m_end)
        {
        }

        iterator& operator++()
        {
            if(m_idx >= m_ref.m_end)
                throw std::out_of_range("no more elements");

            m_idx++;

            return *this;
        }

        auto operator*()
        {
            return m_ref.m_extractor[m_idx];
        }

        bool operator==(iterator const& other) const
        {
            return other.m_idx == m_idx;
        }

        bool operator!=(iterator const& other) const
        {
            return other.m_idx != m_idx;
        }

      private:
        container<T> m_ref;
        jsize        m_idx;
    };

    iterator begin()
    {
        return iterator(*this, 0);
    }

    iterator end()
    {
        return iterator(*this);
    }

  private:
    extract_type<T> m_extractor;

    jsize m_end;
};

} // namespace array_extractors
