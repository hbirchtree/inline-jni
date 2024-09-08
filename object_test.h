#pragma once

#include "errors.h"
#include "jni_types.h"

namespace jnipp::java::objects {

inline void verify_instance_of(jobject instance, std::string const& className)
{
    if(!instance)
        throw java_exception("null object");

    auto classId = GetJNI()->FindClass(className.c_str());

    if(GetJNI()->IsInstanceOf(instance, classId) == JNI_FALSE)
    {
        throw java_type_cast_exception("invalid cast");
    }
}

inline bool not_null(optional<java::object> instance)
{
    return instance.has_value() && instance->instance;
}

} // namespace jnipp::java::objects
