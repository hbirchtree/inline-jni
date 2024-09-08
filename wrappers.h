#pragma once

#include "jni_types.h"

namespace jnipp::java {

template<typename T>
struct type_wrapper
{
    type_wrapper(T)
    {
    }

    operator java::value()
    {
        return java::value();
    }
};

template<>
struct type_wrapper<std::string>
{
    type_wrapper(std::string const& value)
        : value(value)
    {
    }

    operator jvalue() const
    {
        jvalue  out;
        jstring val = GetJNI()->NewStringUTF(value.c_str());
        out.l       = val;
        return out;
    }

    std::string value;
};

#define TYPE_WRAPPER(JTYPE, JV_MEMBER) \
    template<>                         \
    struct type_wrapper<JTYPE>         \
    {                                  \
        type_wrapper(JTYPE value)      \
            : value(value)             \
        {                              \
        }                              \
        operator jvalue() const        \
        {                              \
            jvalue v  = {};            \
            JV_MEMBER = value;         \
            return v;                  \
        }                              \
        JTYPE value;                   \
    };

TYPE_WRAPPER(jobject, v.l)

TYPE_WRAPPER(jboolean, v.z)
TYPE_WRAPPER(jbyte, v.b)
TYPE_WRAPPER(jchar, v.c)
TYPE_WRAPPER(jshort, v.s)
TYPE_WRAPPER(jint, v.i)
TYPE_WRAPPER(jlong, v.j)

TYPE_WRAPPER(jfloat, v.f)
TYPE_WRAPPER(jdouble, v.d)

#undef TYPE_WRAPPER

} // namespace jnipp::java
