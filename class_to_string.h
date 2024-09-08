#pragma once

#include "class_constructor.h"
#include "unwrappers.h"

#include "method_call_impl.h"

namespace jnipp {

inline std::string get_class_name(java::object instance)
{
    auto Object = jnipp::get_class({"java.lang.Object"});

    auto getClass = wrapping::jmethod<return_type::void_>({"getClass"})
                        .ret("java.lang.Class");
    auto getName = wrapping::jmethod<return_type::void_>({"getName"})
                       .ret("java.lang.String");

    auto objectInstance = Object(instance);
    auto classObject    = objectInstance[getClass]();
    auto className      = classObject[getName]();

    return java::type_unwrapper<std::string>(className);
}

} // namespace jnipp
