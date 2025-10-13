#pragma once

#include "arrays.h"
#include "jni_types.h"
#include "object_test.h"

namespace jnipp::java {

template<typename T>
struct type_unwrapper
{
    type_unwrapper(java::value value)
        : value(value)
    {
    }

    operator T() const
    {
        return T();
    }

    java::value value;
};

template<>
struct type_unwrapper<std::string>
{
    type_unwrapper(java::object value)
        : value(value)
    {
    }

    operator std::string() const
    {
        objects::verify_instance_of(value, "java/lang/String");

        jstring str_obj =
            reinterpret_cast<jstring>(static_cast<::jobject>(value));

        auto        chars = GetJNI()->GetStringUTFChars(str_obj, nullptr);
        std::string out;
        if(chars)
            out = chars;

        GetJNI()->ReleaseStringUTFChars(str_obj, chars);

        return out;
    }

    java::object value;
};

template<return_type T>
struct array_type_unwrapper
{
    array_type_unwrapper(java::object obj)
        : arrayRef(*obj.array())
    {
    }

    array_type_unwrapper(java::array arr)
        : arrayRef(arr)
    {
    }

    array_extractors::container<T> operator*()
    {
        return array_extractors::container<T>(arrayRef);
    }

    java::array arrayRef;
};

} // namespace jnipp::java
