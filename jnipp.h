#pragma once

#include "jni_types.h"
#include <algorithm>
#include <stdexcept>
#include <vector>

namespace jnipp {

struct java_exception : std::runtime_error
{
    using runtime_error::runtime_error;
};

struct java_type_cast_exception : std::runtime_error
{
    using runtime_error::runtime_error;
};

extern JNIEnv* GetJNI();

namespace java {

namespace objects {

inline void verify_instance_of(jobject instance, std::string const& className)
{
    auto classId = GetJNI()->FindClass(className.c_str());

    if(GetJNI()->IsInstanceOf(instance, classId) == JNI_FALSE)
    {
        throw java_type_cast_exception("invalid cast");
    }
}

} // namespace objects

template<>
struct type_unwrapper<std::string>
{
    type_unwrapper(jvalue value) : value(value)
    {
    }

    operator std::string() const
    {
        objects::verify_instance_of(value.l, "java/lang/String");

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

#define TYPE_UNWRAPPER(OUT_TYPE, JV_MEMBER)         \
    template<>                                      \
    struct type_unwrapper<OUT_TYPE>                 \
    {                                               \
        type_unwrapper(jvalue value) : value(value) \
        {                                           \
        }                                           \
        operator OUT_TYPE() const                   \
        {                                           \
            return JV_MEMBER;                       \
        }                                           \
        jvalue value;                               \
    };

TYPE_UNWRAPPER(jobject, value.l)

TYPE_UNWRAPPER(jboolean, value.z)
TYPE_UNWRAPPER(jbyte, value.b)
TYPE_UNWRAPPER(jchar, value.c)
TYPE_UNWRAPPER(jshort, value.s)
TYPE_UNWRAPPER(jint, value.i)
TYPE_UNWRAPPER(jlong, value.j)

TYPE_UNWRAPPER(jfloat, value.f)
TYPE_UNWRAPPER(jdouble, value.d)

#undef TYPE_UNWRAPPER

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

#define TYPE_WRAPPER(JTYPE, JV_MEMBER)           \
    template<>                                   \
    struct type_wrapper<JTYPE>                   \
    {                                            \
        type_wrapper(JTYPE value) : value(value) \
        {                                        \
        }                                        \
        operator jvalue() const                  \
        {                                        \
            jvalue v  = {};                      \
            JV_MEMBER = value;                   \
            return v;                            \
        }                                        \
        JTYPE value;                             \
    };

TYPE_WRAPPER(jobject, v.l)
TYPE_WRAPPER(jstring, v.l)

TYPE_WRAPPER(jboolean, v.z)
TYPE_WRAPPER(jbyte, v.b)
TYPE_WRAPPER(jchar, v.c)
TYPE_WRAPPER(jshort, v.s)
TYPE_WRAPPER(jint, v.i)
TYPE_WRAPPER(jlong, v.j)

TYPE_WRAPPER(jfloat, v.f)
TYPE_WRAPPER(jdouble, v.d)

#undef TYPE_WRAPPER

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

template<
    typename T,
    typename std::enable_if<std::is_same<T, jvalue>::value>::type* = nullptr>
inline void get_arg_value(std::vector<jvalue>& values, T arg1)
{
    values.push_back(arg1);
}

template<
    typename T,
    typename std::enable_if<!std::is_same<T, jvalue>::value>::type* = nullptr>
inline void get_arg_value(std::vector<jvalue>& values, T arg1)
{
    java::type_wrapper<T> wrapper(arg1);
    values.push_back(wrapper);
}

inline void get_arg_list(std::vector<jvalue>&)
{
}

template<typename T, typename... Args>
inline void get_arg_list(std::vector<jvalue>& values, T arg1, Args... args)
{
    get_arg_value(values, arg1);

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

struct java_exception_clear
{
    ~java_exception_clear()
    {
        GetJNI()->ExceptionClear();
    }
};

void check_exception();

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
    typename std::enable_if<std::is_same<RType, void>::value>::type* = nullptr>
inline ::jvalue statically(jclass clazz, jmethodID methodId, Args... args)
{
    std::vector<jvalue> values = arguments::get_args(args...);

    GetJNI()->CallStaticVoidMethodA(clazz, methodId, values.data());

    check_exception();

    return ::jvalue();
}

#define FUNCTION_CALL(                                                         \
    FUNC_TYPE, JTYPE, OUT_VAR, OUT_TYPE, SIGNATURE, FIRST_ARG_TYPE, FIRST_ARG) \
    template<                                                                  \
        typename RType,                                                        \
        typename... Args,                                                      \
        typename std::enable_if<std::is_same<RType, JTYPE>::value>::type* =    \
            nullptr>                                                           \
    inline OUT_TYPE SIGNATURE(                                                 \
        FIRST_ARG_TYPE FIRST_ARG, jmethodID methodId, Args... args)            \
    {                                                                          \
        std::vector<jvalue> values = arguments::get_args(args...);             \
        jvalue              out;                                               \
        OUT_VAR = GetJNI()->Call##FUNC_TYPE##MethodA(                          \
            FIRST_ARG, methodId, values.data());                               \
        check_exception();                                                     \
        return out;                                                            \
    }

#define INSTANCE_CALL(FUNC_TYPE, JTYPE, OUT_VAR, OUT_TYPE) \
    FUNCTION_CALL(                                         \
        FUNC_TYPE, JTYPE, OUT_VAR, OUT_TYPE, instanced, jobject, instance)
#define INSTANCE_VALUE_CALL(FUNC_TYPE, JTYPE, OUT_VAR) \
    INSTANCE_CALL(FUNC_TYPE, JTYPE, OUT_VAR, ::jvalue)

#define STATIC_CALL(FUNC_TYPE, JTYPE, OUT_VAR, OUT_TYPE) \
    FUNCTION_CALL(                                       \
        Static##FUNC_TYPE,                               \
        JTYPE,                                           \
        OUT_VAR,                                         \
        OUT_TYPE,                                        \
        statically,                                      \
        jclass,                                          \
        clazz)
#define STATIC_VALUE_CALL(FUNC_TYPE, JTYPE, OUT_VAR) \
    STATIC_CALL(FUNC_TYPE, JTYPE, OUT_VAR, ::jvalue)

#define STATIC_INSTANCE_PAIR(FUNC_TYPE, JTYPE, OUT_VAR) \
    STATIC_VALUE_CALL(FUNC_TYPE, JTYPE, OUT_VAR)        \
    INSTANCE_VALUE_CALL(FUNC_TYPE, JTYPE, OUT_VAR)

STATIC_INSTANCE_PAIR(Object, jobject, out.l)

STATIC_INSTANCE_PAIR(Boolean, jboolean, out.z)
STATIC_INSTANCE_PAIR(Byte, jbyte, out.b)
STATIC_INSTANCE_PAIR(Char, jchar, out.c)
STATIC_INSTANCE_PAIR(Short, jshort, out.s)
STATIC_INSTANCE_PAIR(Int, jint, out.i)
STATIC_INSTANCE_PAIR(Long, jlong, out.j)

STATIC_INSTANCE_PAIR(Float, jfloat, out.f)
STATIC_INSTANCE_PAIR(Double, jdouble, out.d)

#undef FUNCTION_CALL
#undef STATIC_CALL
#undef STATIC_VALUE_CALL
#undef INSTANCE_CALL
#undef INSTANCE_VALUE_CALL
#undef STATIC_INSTANCE_PAIR

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

        invocation::call::check_exception();

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

/* Primitive types */

template<
    typename T,
    typename std::enable_if<std::is_same<T, jboolean>::value>::type* = nullptr>
inline std::string to_str()
{
    return "Z";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jbyte>::value>::type* = nullptr>
inline std::string to_str()
{
    return "B";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jchar>::value>::type* = nullptr>
inline std::string to_str()
{
    return "B";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jshort>::value>::type* = nullptr>
inline std::string to_str()
{
    return "S";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jint>::value>::type* = nullptr>
inline std::string to_str()
{
    return "I";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jlong>::value>::type* = nullptr>
inline std::string to_str()
{
    return "J";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jfloat>::value>::type* = nullptr>
inline std::string to_str()
{
    return "F";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jdouble>::value>::type* = nullptr>
inline std::string to_str()
{
    return "D";
}

/* Array types */

template<
    typename T,
    typename std::enable_if<std::is_same<T, jbooleanArray>::value>::type* =
        nullptr>
inline std::string to_str()
{
    return "[Z";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jbyteArray>::value>::type* =
        nullptr>
inline std::string to_str()
{
    return "[B";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jcharArray>::value>::type* =
        nullptr>
inline std::string to_str()
{
    return "[C";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jshortArray>::value>::type* =
        nullptr>
inline std::string to_str()
{
    return "[S";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jintArray>::value>::type* = nullptr>
inline std::string to_str()
{
    return "[I";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jlongArray>::value>::type* =
        nullptr>
inline std::string to_str()
{
    return "[J";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jfloatArray>::value>::type* =
        nullptr>
inline std::string to_str()
{
    return "[F";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jdoubleArray>::value>::type* =
        nullptr>
inline std::string to_str()
{
    return "[D";
}

template<
    typename T,
    typename std::enable_if<std::is_same<T, jobjectArray>::value>::type* =
        nullptr>
inline std::string to_str()
{
    return "[Ljava/lang/Object;";
}

/* Exceptional types */

template<
    typename T,
    typename std::enable_if<std::is_same<T, void>::value>::type* = nullptr>
inline std::string to_str()
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

        invocation::call::check_exception();

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

        invocation::call::check_exception();

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

        invocation::call::check_exception();

        return {java::method_reference({object.instance, methodId})};
    }

    template<typename T>
    class_props::instance_field<T> operator[](jfield<T> const& field)
    {
        auto fieldId =
            GetJNI()->GetFieldID(object.clazz, field.name(), field.signature());

        invocation::call::check_exception();

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

namespace java {
namespace objects {

inline std::string get_class(jobject instance)
{
    auto Object = jnipp::get_class({"java.lang.Object"});
    auto Class  = jnipp::get_class({"java.lang.Class"});

    auto getClass =
        wrapping::jmethod<void>({"getClass"}).ret("java.lang.Class");
    auto getName = wrapping::jmethod<void>({"getName"}).ret("java.lang.String");

    auto objectInstance = Object(instance);
    auto classObject    = objectInstance[getClass]();
    auto className      = Class(classObject.l)[getName]();

    return java::type_unwrapper<std::string>(className);
}

} // namespace objects
} // namespace java

} // namespace jnipp

namespace jnipp_operators {

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

} // namespace jnipp_operators

inline void jnipp::invocation::call::check_exception()
{
    if(GetJNI()->ExceptionCheck() == JNI_TRUE)
    {
        java_exception_clear _;

        ::jthrowable exc = GetJNI()->ExceptionOccurred();

        GetJNI()->ExceptionClear();

        auto exceptionType = jnipp::java::objects::get_class(exc);

        jclass    clazz = GetJNI()->FindClass("java/lang/Throwable");
        jmethodID method =
            GetJNI()->GetMethodID(clazz, "getMessage", "()Ljava/lang/String;");

        jobject message = GetJNI()->CallObjectMethod(exc, method);

        java::objects::verify_instance_of(message, "java/lang/String");

        auto messageBytes = GetJNI()->GetStringUTFChars(
            reinterpret_cast<jstring>(message), nullptr);

        if(!messageBytes)
            throw java_exception("exception occurred with no message");

        std::string messageCopy = messageBytes;

        GetJNI()->ReleaseStringUTFChars(
            reinterpret_cast<jstring>(message), messageBytes);

        throw java_exception(exceptionType + ": " + messageCopy);
    }
}
