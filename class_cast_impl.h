#pragma once

#include "class_constructor.h"
#include "class_wrapper.h"

namespace jnipp::wrapping {

inline jobject jobject::cast(jclass const& clazz) const
{
    auto cpy         = *this;
    cpy.object.clazz = get_class(clazz.clazz).clazz;
    return cpy;
}

} // namespace jnipp::wrapping
