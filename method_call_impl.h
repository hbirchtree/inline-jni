#pragma once

#include "class_constructor.h"
#include "class_wrapper.h"
#include "unwrappers.h"

namespace jnipp::invocation::call {

namespace {

constexpr return_type array_type_to_value_type(return_type t)
{
    switch(t)
    {
        // clang-format off
    case return_type::bool_array_: return return_type::bool_;
    case return_type::byte_array_: return return_type::byte_;
    case return_type::char_array_: return return_type::char_;
    case return_type::short_array_: return return_type::short_;
    case return_type::int_array_: return return_type::int_;
    case return_type::long_array_: return return_type::long_;
    case return_type::float_array_: return return_type::float_;
    case return_type::double_array_: return return_type::double_;
    default: return return_type::void_;
        // clang-format on
    }
}

} // namespace

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
    } else if constexpr(Type == return_type::object_array_)
    {
        auto out = call_no_except<Type, Calling>(
            clazz, obj, method, std::forward<Args>(args)...);
        check_exception();
        if(!method.return_class.has_value())
            throw std::runtime_error("no return class provided");
        auto output_class = get_class(java::clazz(*method.return_class));
        return java::array_type_unwrapper<return_type::object_>(java::array{
            .instance    = *out.array(),
            .value_class = output_class.clazz,
            .value_type  = *method.return_class,
        });
    } else if constexpr(
        Type == return_type::bool_array_ || Type == return_type::byte_array_ ||
        Type == return_type::char_array_ || Type == return_type::short_array_ ||
        Type == return_type::int_array_ || Type == return_type::long_array_ ||
        Type == return_type::float_array_ || Type == return_type::double_array_)
    {
        auto out = call_no_except<Type, Calling>(
            clazz, obj, method, std::forward<Args>(args)...);
        check_exception();
        return java::array_type_unwrapper<array_type_to_value_type(Type)>(
            java::array{
                .instance = out,
            });
    } else if constexpr(Type != return_type::void_)
    {
        auto out = call_no_except<Type, Calling>(
            clazz, obj, method, std::forward<Args>(args)...);
        check_exception();
        return out;
    }
}

} // namespace jnipp::invocation::call
