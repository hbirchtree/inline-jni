#pragma once

#include "arrays.h"
#include "errors.h"
#include "field_access.h"
#include "jni_types.h"
#include "method_calls.h"
#include "unwrappers.h"
#include "wrappers.h"

#include "class_cast_impl.h"
#include "class_to_string.h"
#include "field_access_impl.h"
#include "method_call_impl.h"

#include <algorithm>
#include <peripherals/stl/any_of.h>
#include <stdexcept>
#include <vector>

namespace jnipp::literals {

FORCEDINLINE jnipp::wrapping::jclass operator""_jclass(
    const char* name, size_t)
{
    return jnipp::get_class({name});
}

FORCEDINLINE jnipp::wrapping::jmethod<jnipp::return_type::void_>
operator""_jmethod(const char* name, size_t)
{
    return {jnipp::java::method{name}};
}

FORCEDINLINE jnipp::wrapping::jfield<jnipp::return_type::void_>
operator""_jfield(const char* name, size_t)
{
    return {jnipp::java::field{name}};
}

} // namespace jnipp::literals

inline void jnipp::invocation::call::check_exception()
{
    if(GetJNI()->ExceptionCheck() == JNI_TRUE)
    {
        exception_clear_scope _;

        auto exception = java::object({}, GetJNI()->ExceptionOccurred());

        GetJNI()->ExceptionClear();

        auto exceptionType =
            get_class_name(java::object(java::clazz{}, exception));

        auto Throwable = get_class(java::clazz{"java.lang.Throwable"});
        auto getMessage =
            wrapping::jmethod<return_type::void_>(java::method("getMessage"))
                .ret("java.lang.String");

        auto message = java::type_unwrapper<std::string>(
            Throwable(exception)[getMessage]());

        throw java_exception(
            exceptionType + ": " + static_cast<std::string>(message));
    }
}
