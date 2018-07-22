#pragma once

#include "jni_types.h"
#include <stdexcept>
#include <algorithm>
#include <vector>

namespace jnipp {

struct java_exception : std::runtime_error
{
    using runtime_error::runtime_error;
};

extern JNIEnv* GetJNI();

namespace java {

template<>
struct type_unwrapper<std::string>
{
    type_unwrapper(jvalue value) : value(value)
    {
    }

    operator std::string() const
    {
        jstring str_obj = reinterpret_cast<jstring>(value.l);

        auto        chars = GetJNI()->GetStringUTFChars(str_obj, nullptr);
        std::string out;
        if(chars)
            out = chars;
        GetJNI()->ReleaseStringUTFChars(str_obj, chars);

        return out;
    }

    jvalue value;
};

template<>
struct type_wrapper<std::string>
{
    type_wrapper(std::string const& value) : value(value)
    {
    }

    operator jvalue() const
    {
        jvalue  out;
        jstring val = GetJNI()->NewStringUTF(value.c_str());
        out.l       = val;
        return out;
    }

    std::string value;
};

template<>
struct type_wrapper<jvalue>
{
    type_wrapper(jvalue value) : value(value)
    {
    }

    operator jvalue() const
    {
        return value;
    }

    jvalue value;
};

} // namespace java

namespace invocation {

template<typename RType, typename... Args>
struct instance_call;

template<typename RType, typename... Args>
struct static_call;

} // namespace invocation

namespace invocation {

template<typename ArgType, typename InputType>
struct arg_pair
{
    using arg_type = ArgType;
    using in_type  = InputType;
};

namespace arguments {

template<typename T>
inline void get_arg_list(std::vector<jvalue>& values, T arg1)
{
    java::type_wrapper<T> from(arg1);
    values.push_back(from);
}

inline void get_arg_list(std::vector<jvalue>&)
{
}

template<typename T, typename... Args>
inline void get_arg_list(std::vector<jvalue>& values, T arg1, Args... args)
{
    get_arg_list(values, arg1);

    get_arg_list(values, args...);
}

template<typename... Args>
inline std::vector<jvalue> get_args(Args... args)
{
    std::vector<jvalue> out;
    get_arg_list(out, args...);

    return out;
}

} // namespace arguments

namespace call {

inline void check_exception()
{
    if(GetJNI()->ExceptionCheck() == JNI_TRUE)
    {
        ::jthrowable exc = GetJNI()->ExceptionOccurred();

        GetJNI()->ExceptionClear();

        jclass    clazz = GetJNI()->FindClass("java/lang/Throwable");
        jmethodID method =
            GetJNI()->GetMethodID(clazz, "getMessage", "()Ljava/lang/String;");

        jobject message = GetJNI()->CallObjectMethod(exc, method);

        auto messageBytes = GetJNI()->GetStringUTFChars(
            reinterpret_cast<jstring>(message), nullptr);

        if(!messageBytes)
        {
            GetJNI()->ExceptionClear();
            throw java_exception("");
        }

        std::string messageCopy = messageBytes;

        GetJNI()->ReleaseStringUTFChars(
            reinterpret_cast<jstring>(message), messageBytes);

        GetJNI()->ExceptionClear();
        throw java_exception(messageCopy);
    }
}

template<
    typename RType,
    typename... Args,
    typename std::enable_if<std::is_same<RType, void>::value>::type* = nullptr>
inline ::jvalue instanced(jobject clazz, jmethodID methodId, Args... args)
{
    std::vector<jvalue> values = arguments::get_args(args...);

    GetJNI()->CallVoidMethodA(clazz, methodId, values.data());

    check_exception();

    return ::jvalue();
}

template<
    typename RType,
    typename... Args,
    typename std::enable_if<std::is_same<RType, jobject>::value>::type* =
        nullptr>
inline ::jvalue instanced(jobject clazz, jmethodID methodId, Args... args)
{
    std::vector<jvalue> values = arguments::get_args(args...);

    jvalue out;

    out.l = GetJNI()->CallObjectMethodA(clazz, methodId, values.data());

    check_exception();

    return out;
}

/* TODO: Implement primitive calls */

template<
    typename RType,
    typename... Args,
    typename std::enable_if<std::is_same<RType, void>::value>::type* = nullptr>
inline ::jvalue statically(jclass clazz, jmethodID methodId, Args... args)
{
    std::vector<jvalue> values = arguments::get_args(args...);

    GetJNI()->CallStaticVoidMethodA(clazz, methodId, values.data());

    check_exception();

    return ::jvalue();
}

template<
    typename RType,
    typename... Args,
    typename std::enable_if<std::is_same<RType, jobject>::value>::type* =
        nullptr>
inline ::jvalue statically(jclass clazz, jmethodID methodId, Args... args)
{
    std::vector<jvalue> values = arguments::get_args(args...);

    jvalue out;

    out.l = GetJNI()->CallStaticObjectMethodA(clazz, methodId, values.data());

    check_exception();

    return out;
}

/* TODO: Implement primitive calls */

} // namespace call

template<typename RType, typename... Args>
struct instance_call
{
    instance_call(java::method_reference const& method) : method(method)
    {
    }

    inline jvalue operator()(Args... args)
    {
        return call::instanced<RType, Args...>(
            method.instance, method.methodId, args...);
    }

    java::method_reference method;
};

template<typename RType, typename... Args>
struct static_call
{
    static_call(java::static_method_reference const& method) : method(method)
    {
    }

    inline jvalue operator()(Args... args)
    {
        return call::statically<RType, Args...>(
            method.clazz, method.methodId, args...);
    }

    java::static_method_reference method;
};

} // namespace invocation

namespace class_props {

template<typename T>
struct instance_field
{
    java::field_reference field;
};

template<typename T>
struct static_field
{
    java::static_field_reference field;
};

template<>
struct static_field<jobject>
{
    jvalue operator*()
    {
        jvalue out;

        auto jobject =
            GetJNI()->GetStaticObjectField(field.clazz, field.fieldId);
        out.l = jobject;

        return out;
    }

    java::static_field_reference field;
};

} // namespace class_props

namespace wrapping {

namespace arguments {

template<typename T>
inline std::string to_str()
{
    return "_";
}

template<>
inline std::string to_str<bool>()
{
    return "Z";
}

template<>
inline std::string to_str<char>()
{
    return "B";
}

template<>
inline std::string to_str<short>()
{
    return "S";
}

template<>
inline std::string to_str<int>()
{
    return "I";
}

template<>
inline std::string to_str<long>()
{
    return "J";
}

template<>
inline std::string to_str<float>()
{
    return "F";
}

template<>
inline std::string to_str<double>()
{
    return "D";
}

template<>
inline std::string to_str<void>()
{
    return "V";
}

inline std::string classify(std::string const& type)
{
    std::string out = type;

    std::replace(out.begin(), out.end(), '.', '/');

    return "L" + out + ";";
}

} // namespace arguments

template<typename RType, typename... Args>
struct jmethod
{
    jmethod(java::method&& method) : method(std::move(method))
    {
    }

    template<typename T2>
    jmethod<T2, Args...> ret()
    {
        method.ret(arguments::to_str<T2>());
        return {std::move(method)};
    }

    jmethod<::jobject, Args...> ret(std::string const& ret_type)
    {
        method.ret(arguments::classify(ret_type));
        return {std::move(method)};
    }

    template<typename T2>
    jmethod<RType, Args..., T2> arg()
    {
        method.arg(arguments::to_str<T2>());
        return {std::move(method)};
    }

    template<typename T2>
    jmethod<RType, Args..., T2> arg(std::string const& type)
    {
        method.arg(arguments::classify(type));
        return {std::move(method)};
    }

    jmethod<RType, Args..., ::jvalue> arg(std::string const& type)
    {
        method.arg(arguments::classify(type));
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

template<typename T>
struct jfield
{
    jfield(java::field&& field) : field(std::move(field))
    {
    }

    template<typename T2>
    jfield<T2> as()
    {
        field.signature = arguments::to_str<T2>();
        return {field};
    }

    jfield<jobject> as(std::string const& type)
    {
        field.signature = type;
        std::replace(field.signature.begin(), field.signature.end(), '.', '/');
        field.signature = "L" + field.signature + ";";
        return {std::move(field)};
    }

    const char* name() const
    {
        return field.name.c_str();
    }

    const char* signature() const
    {
        return field.signature.c_str();
    }

    java::field field;
};

struct jobject;

struct jclass
{
    jclass(::jclass clazz) : clazz(clazz)
    {
    }

    jclass(std::string const& clazz) :
        jclass(GetJNI()->FindClass(clazz.c_str()))
    {
    }

    /*!
     * \brief Object instantiation
     * \param instance
     * \return
     */
    jobject operator()(::jobject instance);

    template<typename RType, typename... Args>
    /*!
     * \brief Static method calls
     * \param method
     * \return
     */
    invocation::static_call<RType, Args...> operator[](
        jmethod<RType, Args...> const& method)
    {
        auto methodId = GetJNI()->GetStaticMethodID(
            clazz, method.name(), method.signature());

        return {java::static_method_reference({clazz, methodId})};
    }

    template<typename T>
    /*!
     * \brief Static fields
     * \param field
     * \return
     */
    class_props::static_field<T> operator[](jfield<T> const& field)
    {
        auto fieldId =
            GetJNI()->GetStaticFieldID(clazz, field.name(), field.signature());

        return {java::static_field_reference({clazz, fieldId})};
    }

    ::jclass clazz;
};

struct jobject
{
    jobject(java::object const& object) : object(object)
    {
    }

    jobject(::jclass clazz, ::jobject object) : object({clazz, object})
    {
    }

    template<typename RType, typename... Args>
    invocation::instance_call<RType, Args...> operator[](
        jmethod<RType, Args...> const& method)
    {
        auto methodId = GetJNI()->GetMethodID(
            object.clazz, method.name(), method.signature());

        return {java::method_reference({object.instance, methodId})};
    }

    template<typename T>
    class_props::instance_field<T> operator[](jfield<T> const& field)
    {
        auto fieldId =
            GetJNI()->GetFieldID(object.clazz, field.name(), field.signature());

        return {java::field_reference({object.instance, fieldId})};
    }

    java::object object;
};

inline jobject jclass::operator()(::jobject instance)
{
    return jobject(clazz, instance);
}

} // namespace wrapping

inline wrapping::jclass get_class(java::clazz const& clazz)
{
    std::string class_name = clazz.name;

    std::replace(class_name.begin(), class_name.end(), '.', '/');

    return {GetJNI()->FindClass(class_name.c_str())};
}

} // namespace jnipp

FORCEDINLINE jnipp::wrapping::jclass operator"" _jclass(
    const char* name, size_t)
{
    return jnipp::get_class({name});
}

FORCEDINLINE jnipp::wrapping::jmethod<void> operator"" _jmethod(
    const char* name, size_t)
{
    return {{name}};
}

FORCEDINLINE jnipp::wrapping::jfield<void> operator"" _jfield(
    const char* name, size_t)
{
    return {{name}};
}
