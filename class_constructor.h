#pragma once

#include "class_wrapper.h"

namespace jnipp {

inline wrapping::jclass get_class(java::clazz const& clazz)
{
    std::string class_name = *clazz.name;

    class_name = type_signature::slashify(class_name);

    return {GetJNI()->FindClass(class_name.c_str()), class_name};
}

} // namespace jnipp
