#pragma once

#include "jni_types.h"

#include <stdexcept>

namespace jnipp {

struct java_exception : std::runtime_error
{
    using runtime_error::runtime_error;
};

struct java_type_cast_exception : std::runtime_error
{
    using runtime_error::runtime_error;
};

struct exception_clear_scope
{
    ~exception_clear_scope()
    {
        GetJNI()->ExceptionClear();
    }
};

}
