#pragma once

#include "class_constructor.h"
#include "class_wrapper.h"

namespace jnipp::invocation::call {

template<return_type Type, calling_method Calling, typename... Args>
inline auto call(
    java::clazz clazz, java::object obj, java::method method, Args... args)
{
    if constexpr(Type == return_type::void_)
    {
        call_no_except<Type, Calling>(
            clazz, obj, method, std::forward<Args>(args)...);
        check_exception();
    } else if constexpr(Type == return_type::object_)
    {
        auto out = call_no_except<Type, Calling>(
            clazz, obj, method, std::forward<Args>(args)...);
        check_exception();
        if(!method.return_class.has_value())
            throw std::runtime_error("no return class provided");
        auto output_class = get_class(java::clazz(*method.return_class));
        return output_class(out);
    } else if constexpr(Type != return_type::void_)
    {
        auto out = call_no_except<Type, Calling>(
            clazz, obj, method, std::forward<Args>(args)...);
        check_exception();
        return out;
    }
}

} // namespace jnipp::invocation::call
