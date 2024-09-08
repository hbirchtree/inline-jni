#pragma once

#include "jni_types.h"
#include "object_test.h"
#include "type_signatures.h"
#include "wrappers.h"

#include <peripherals/stl/any_of.h>

namespace jnipp::invocation {

template<typename ArgType, typename InputType>
struct arg_pair
{
    using arg_type = ArgType;
    using in_type  = InputType;
};

namespace arguments {

template<typename T>
requires std::is_same_v<T, jvalue>
inline void get_arg_value(std::vector<jvalue>& values, T arg1)
{
    values.push_back(arg1);
}

template<typename T>
requires std::is_same_v<T, java::object>
inline void get_arg_value(std::vector<jvalue>& values, T arg1)
{
    values.push_back(arg1.instance);
}

template<typename T>
requires(!std::is_same_v<T, jvalue> && !std::is_same_v<T, java::object>)
inline void get_arg_value(std::vector<jvalue>& values, T arg1)
{
    java::type_wrapper<T> wrapper(arg1);
    values.push_back(wrapper);
}

template<typename... Args>
inline std::vector<jvalue> get_args(Args... args)
{
    std::vector<jvalue> out;
    (get_arg_value(out, args), ...);

    return out;
}

} // namespace arguments

namespace call {

enum class calling_method
{
    instanced_,
    static_,
};

void check_exception();

template<return_type Type, calling_method Calling, typename... Args>
inline auto call_no_except(
    java::clazz clazz, java::object object, java::method method, Args... args)
{
    auto values = arguments::get_args(std::forward<Args>(args)...);

    if constexpr(Type == return_type::bool_)
    {
        return Calling == calling_method::static_
                   ? GetJNI()->CallStaticBooleanMethodA(
                         clazz, *method, values.data())
                   : GetJNI()->CallBooleanMethodA(
                         object, *method, values.data());
    } else if constexpr(Type == return_type::byte_)
    {
        return Calling == calling_method::static_
                   ? GetJNI()->CallStaticByteMethodA(
                         clazz, *method, values.data())
                   : GetJNI()->CallByteMethodA(object, *method, values.data());
    } else if constexpr(Type == return_type::char_)
    {
        return Calling == calling_method::static_
                   ? GetJNI()->CallStaticCharMethodA(
                         clazz, *method, values.data())
                   : GetJNI()->CallCharMethodA(object, *method, values.data());
    } else if constexpr(Type == return_type::short_)
    {
        return Calling == calling_method::static_
                   ? GetJNI()->CallStaticShortMethodA(
                         clazz, *method, values.data())
                   : GetJNI()->CallShortMethodA(object, *method, values.data());
    } else if constexpr(Type == return_type::int_)
    {
        return Calling == calling_method::static_
                   ? GetJNI()->CallStaticIntMethodA(
                         clazz, *method, values.data())
                   : GetJNI()->CallIntMethodA(object, *method, values.data());
    } else if constexpr(Type == return_type::long_)
    {
        return Calling == calling_method::static_
                   ? GetJNI()->CallStaticLongMethodA(
                         clazz, *method, values.data())
                   : GetJNI()->CallLongMethodA(object, *method, values.data());
    } else if constexpr(Type == return_type::float_)
    {
        return Calling == calling_method::static_
                   ? GetJNI()->CallStaticFloatMethodA(
                         clazz, *method, values.data())
                   : GetJNI()->CallFloatMethodA(object, *method, values.data());
    } else if constexpr(Type == return_type::double_)
    {
        return Calling == calling_method::static_
                   ? GetJNI()->CallStaticDoubleMethodA(
                         clazz, *method, values.data())
                   : GetJNI()->CallDoubleMethodA(
                         object, *method, values.data());
    } else if constexpr(Type == return_type::object_)
    {
        return java::object{
            java::clazz(nullptr),
            Calling == calling_method::static_
                ? GetJNI()->CallStaticObjectMethodA(
                      clazz, *method, values.data())
                : GetJNI()->CallObjectMethodA(object, *method, values.data())};
    } else if constexpr(stl_types::one_of(
                            Type,
                            return_type::bool_array_,
                            return_type::char_array_,
                            return_type::short_array_,
                            return_type::int_array_,
                            return_type::long_array_,
                            return_type::float_array_,
                            return_type::double_array_))
    {
        return java::object{
            java::clazz(nullptr),
            Calling == calling_method::static_
                ? GetJNI()->CallStaticObjectMethodA(
                      clazz, *method, values.data())
                : GetJNI()->CallObjectMethodA(object, *method, values.data())}
            .array()
            .value();
    } else if constexpr(Type == return_type::void_)
    {
        if constexpr(Calling == calling_method::static_)
            GetJNI()->CallStaticVoidMethodA(clazz, *method, values.data());
        else
            GetJNI()->CallVoidMethodA(object, *method, values.data());
    }
}

template<return_type Type, calling_method Calling, typename... Args>
inline auto call(
    java::clazz clazz, java::object obj, java::method method, Args... args);

} // namespace call

template<return_type RType, typename... Args>
struct instance_call
{
    instance_call(java::method_reference const& method)
        : method(method)
    {
    }

    inline auto operator()(Args... args)
    {
        if constexpr(RType == return_type::void_)
            call::call<RType, call::calling_method::instanced_>(
                {},
                method.instance,
                method.method,
                std::forward<Args>(args)...);
        else
            return call::call<RType, call::calling_method::instanced_>(
                {},
                method.instance,
                method.method,
                std::forward<Args>(args)...);
    }

    java::method_reference method;
};

template<return_type RType, typename... Args>
struct static_call
{
    static_call(java::static_method_reference const& method)
        : method(method)
    {
    }

    inline auto operator()(Args... args)
    {
        if constexpr(RType == return_type::void_)
            call::call<RType, call::calling_method::static_>(
                method.clazz, {}, method.method, std::forward<Args>(args)...);
        else
            return call::call<RType, call::calling_method::static_>(
                method.clazz, {}, method.method, std::forward<Args>(args)...);
    }

    java::static_method_reference method;
};

template<typename... Args>
struct constructor_call
{
    constructor_call(java::static_method_reference const& method)
        : method(method)
    {
    }

    inline optional<java::object> operator()(Args... args)
    {
        std::vector<jvalue> values = arguments::get_args(args...);

        auto instance =
            GetJNI()->NewObject(method.clazz, *method.method, values.data());

        java::object out(method.clazz, instance);

        if(!java::objects::not_null(out))
            return {};

        return java::object{method.clazz, instance};
    }

    java::static_method_reference method;
};

} // namespace jnipp::invocation

namespace jnipp::wrapping {

template<return_type RType, typename... Args>
struct jmethod
{
    jmethod(java::method&& method)
        : method(std::move(method))
    {
    }

    template<return_type T2>
    /*!
     * \brief define a POD or POD array return type
     * \return
     */
    jmethod<T2, Args...> ret()
    {
        method.ret(type_signature::to_str<T2>());
        return {std::move(method)};
    }

    /*!
     * \brief define an object return type
     * \param ret_type Java class name for return type
     * \return
     */
    jmethod<return_type::object_, Args...> ret(std::string const& ret_type)
    {
        method.ret(type_signature::classify(ret_type));
        method.return_class = ret_type;
        return {std::move(method)};
    }

    template<return_type T2>
    requires(T2 == return_type::object_array_)
    /*!
     * \brief define an object array return type
     * \param type Java class name of returned type
     * \return
     */
    jmethod<return_type::object_, Args...> ret(std::string const& type)
    {
        method.ret(type_signature::to_str<T2>(type));
        method.return_class = type;
        return {std::move(method)};
    }

    template<typename T2>
    /*!
     * \brief define argument with POD or POD array type
     * \return
     */
    jmethod<RType, Args..., T2> arg()
    {
        method.arg(type_signature::to_str<T2>());
        return {std::move(method)};
    }

    template<typename T2>
    /*!
     * \brief define custom argument type, will call on
     * jnipp::java::type_wrapper<T2> to translate argument
     * \param type Java class name for argument, or JNI POD type
     * \return
     */
    jmethod<RType, Args..., T2> arg(std::string const& type)
    {
        method.arg(type_signature::classify(type));
        return {std::move(method)};
    }

    /*!
     * \brief define class or POD argument type using jvalue
     * \param type
     * \return
     */
    jmethod<RType, Args..., ::jvalue> arg(std::string const& type)
    {
        method.arg(type_signature::classify(type));
        return {std::move(method)};
    }

    const char* name() const
    {
        return method.name.c_str();
    }

    const char* signature() const
    {
        return method.signature.c_str();
    }

    java::method method;
};

} // namespace jnipp::wrapping
