#pragma once

#include "jni_types.h"
#include <algorithm>
#include <peripherals/stl/any_of.h>
#include <stdexcept>
#include <vector>

namespace jnipp {

template<typename T>
struct is_jarray
{
    static constexpr bool value =
        std::is_base_of<_jarray, typename std::remove_pointer<T>::type>::value;
};

struct java_exception : std::runtime_error
{
    using runtime_error::runtime_error;
};

struct java_type_cast_exception : std::runtime_error
{
    using runtime_error::runtime_error;
};

enum class return_type
{
    byte_,
    char_,
    short_,
    int_,
    long_,
    bool_,
    float_,
    double_,

    byte_array_,
    char_array_,
    short_array_,
    int_array_,
    long_array_,
    bool_array_,
    float_array_,
    double_array_,
    object_array_,

    array_,
    object_,
    void_,
};

namespace java {

namespace objects {

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

} // namespace objects

struct exception_clear_scope
{
    ~exception_clear_scope()
    {
        GetJNI()->ExceptionClear();
    }
};

namespace array_extractors {

namespace detail {

template<return_type T>
inline auto get_element(::jarray, ::jsize)
{
}

template<>
inline auto get_element<return_type::object_>(::jarray instance, ::jsize i)
{
    return GetJNI()->GetObjectArrayElement(
        reinterpret_cast<jobjectArray>(instance), i);
}

#define DEFINE_GET_ELEMENT(JAVA_TYPE, JAVA_NAME, RETURN_TYPE)          \
    template<>                                                         \
    inline auto get_element<RETURN_TYPE>(::jarray instance, ::jsize i) \
    {                                                                  \
        JAVA_TYPE out;                                                 \
        GetJNI()->Get##JAVA_NAME##ArrayRegion(                         \
            reinterpret_cast<JAVA_TYPE##Array>(instance), i, 1, &out); \
        return out;                                                    \
    }

DEFINE_GET_ELEMENT(::jboolean, Boolean, return_type::bool_)
DEFINE_GET_ELEMENT(::jbyte, Byte, return_type::byte_)
DEFINE_GET_ELEMENT(::jchar, Char, return_type::char_)
DEFINE_GET_ELEMENT(::jshort, Short, return_type::short_)
DEFINE_GET_ELEMENT(::jint, Int, return_type::int_)
DEFINE_GET_ELEMENT(::jlong, Long, return_type::long_)
DEFINE_GET_ELEMENT(::jfloat, Float, return_type::float_)
DEFINE_GET_ELEMENT(::jdouble, Double, return_type::double_)

#undef DEFINE_GET_ELEMENT

} // namespace detail

template<return_type T>
struct extract_type
{
    extract_type(java::array array)
        : ref(array)
    {
    }

    jlong length() const
    {
        return ref.length();
    }

    auto operator[](jsize index)
    {
        if(index >= length())
            throw std::out_of_range(
                std::to_string(index) + " >= " + std::to_string(length()));

        if constexpr(T == return_type::object_)
            return java::object{
                {},
                GetJNI()->GetObjectArrayElement(
                    reinterpret_cast<jobjectArray>(ref.instance), index)};
        else if constexpr(T != return_type::object_)
            return detail::get_element<T>(ref.instance, index);
        else
            return java::value();
    }

    java::array ref;
};

template<return_type T>
struct container
{
    container(java::array arrayObject)
        : m_extractor(arrayObject)
        , m_end(m_extractor.length())
    {
    }

    struct iterator
    {
        iterator(container<T>& container, jsize idx)
            : m_ref(container)
            , m_idx(idx)
        {
        }

        iterator(container<T>& container)
            : m_ref(container)
            , m_idx(m_ref.m_end)
        {
        }

        iterator& operator++()
        {
            if(m_idx >= m_ref.m_end)
                throw std::out_of_range("no more elements");

            m_idx++;

            return *this;
        }

        auto operator*()
        {
            return m_ref.m_extractor[m_idx];
        }

        bool operator==(iterator const& other) const
        {
            return other.m_idx == m_idx;
        }

        bool operator!=(iterator const& other) const
        {
            return other.m_idx != m_idx;
        }

      private:
        container<T> m_ref;
        jsize        m_idx;
    };

    iterator begin()
    {
        return iterator(*this, 0);
    }

    iterator end()
    {
        return iterator(*this);
    }

  private:
    extract_type<T> m_extractor;

    jsize m_end;
};

} // namespace array_extractors

template<>
struct type_unwrapper<std::string>
{
    type_unwrapper(java::object value)
        : value(value)
    {
    }

    operator std::string() const
    {
        objects::verify_instance_of(value, "java/lang/String");

        jstring str_obj =
            reinterpret_cast<jstring>(static_cast<::jobject>(value));

        auto        chars = GetJNI()->GetStringUTFChars(str_obj, nullptr);
        std::string out;
        if(chars)
            out = chars;

        GetJNI()->ReleaseStringUTFChars(str_obj, chars);

        return out;
    }

    java::object value;
};

template<>
struct type_wrapper<std::string>
{
    type_wrapper(std::string const& value)
        : value(value)
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

#define TYPE_WRAPPER(JTYPE, JV_MEMBER) \
    template<>                         \
    struct type_wrapper<JTYPE>         \
    {                                  \
        type_wrapper(JTYPE value)      \
            : value(value)             \
        {                              \
        }                              \
        operator jvalue() const        \
        {                              \
            jvalue v  = {};            \
            JV_MEMBER = value;         \
            return v;                  \
        }                              \
        JTYPE value;                   \
    };

TYPE_WRAPPER(jobject, v.l)

TYPE_WRAPPER(jboolean, v.z)
TYPE_WRAPPER(jbyte, v.b)
TYPE_WRAPPER(jchar, v.c)
TYPE_WRAPPER(jshort, v.s)
TYPE_WRAPPER(jint, v.i)
TYPE_WRAPPER(jlong, v.j)

TYPE_WRAPPER(jfloat, v.f)
TYPE_WRAPPER(jdouble, v.d)

#undef TYPE_WRAPPER

template<return_type T>
struct array_type_unwrapper
{
    array_type_unwrapper(java::object obj)
        : arrayRef(*obj.array())
    {
    }

    array_extractors::container<T> operator*()
    {
        return array_extractors::container<T>(arrayRef);
    }

    java::array arrayRef;
};

} // namespace java

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
    java::clazz clazz, java::object obj, java::method method, Args... args)
{
    if constexpr(Type == return_type::void_)
    {
        call_no_except<Type, Calling>(
            clazz, obj, method, std::forward<Args>(args)...);
        check_exception();
    } else if constexpr(Type != return_type::void_)
    {
        auto out = call_no_except<Type, Calling>(
            clazz, obj, method, std::forward<Args>(args)...);
        check_exception();
        return out;
    }
}

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

} // namespace invocation

namespace class_props {

template<return_type T>
struct instance_field
{
    auto operator*() const
    {
        if constexpr(T == return_type::bool_)
            return GetJNI()->GetBooleanField(field.instance, *field.field);
        else if constexpr(T == return_type::byte_)
            return GetJNI()->GetByteField(field.instance, *field.field);
        else if constexpr(T == return_type::char_)
            return GetJNI()->GetCharField(field.instance, *field.field);
        else if constexpr(T == return_type::short_)
            return GetJNI()->GetShortField(field.instance, *field.field);
        else if constexpr(T == return_type::int_)
            return GetJNI()->GetIntField(field.instance, *field.field);
        else if constexpr(T == return_type::long_)
            return GetJNI()->GetLongField(field.instance, *field.field);
        else if constexpr(T == return_type::float_)
            return GetJNI()->GetFloatField(field.instance, *field.field);
        else if constexpr(T == return_type::double_)
            return GetJNI()->GetDoubleField(field.instance, *field.field);
        else if constexpr(T == return_type::object_)
            return java::object{
                {}, GetJNI()->GetObjectField(field.instance, *field.field)};
        else
            return java::value();
    }

    java::field_reference field;
};

template<return_type T>
struct static_field
{
    auto operator*() const
    {
        if constexpr(T == return_type::bool_)
            return GetJNI()->GetStaticBooleanField(field.clazz, *field.field);
        else if constexpr(T == return_type::byte_)
            return GetJNI()->GetStaticByteField(field.clazz, *field.field);
        else if constexpr(T == return_type::char_)
            return GetJNI()->GetStaticCharField(field.clazz, *field.field);
        else if constexpr(T == return_type::short_)
            return GetJNI()->GetStaticShortField(field.clazz, *field.field);
        else if constexpr(T == return_type::int_)
            return GetJNI()->GetStaticIntField(field.clazz, *field.field);
        else if constexpr(T == return_type::long_)
            return GetJNI()->GetStaticLongField(field.clazz, *field.field);
        else if constexpr(T == return_type::float_)
            return GetJNI()->GetStaticFloatField(field.clazz, *field.field);
        else if constexpr(T == return_type::double_)
            return GetJNI()->GetStaticDoubleField(field.clazz, *field.field);
        else if constexpr(T == return_type::object_)
            return java::object{
                {}, GetJNI()->GetStaticObjectField(field.clazz, *field.field)};
        else
            return java::value();
    }

    java::static_field_reference field;
};

} // namespace class_props

namespace wrapping {

namespace arguments {

inline std::string slashify(std::string const& type)
{
    auto out = type;
    std::replace(out.begin(), out.end(), '.', '/');
    return out;
}

inline std::string classify(std::string const& type)
{
    std::string out = type;

    out = slashify(type);

    return "L" + out + ";";
}

template<typename T, typename std::enable_if<false, T>::type* = nullptr>
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
inline std::string to_str(std::string const& c)
{
    return "[L" + slashify(c) + ";";
}

template<return_type T>
inline std::string to_str(std::string const& c = {})
{
    if constexpr(T == return_type::bool_)
        return "Z";
    else if constexpr(T == return_type::byte_)
        return "B";
    else if constexpr(T == return_type::char_)
        return "C";
    else if constexpr(T == return_type::short_)
        return "S";
    else if constexpr(T == return_type::int_)
        return "I";
    else if constexpr(T == return_type::long_)
        return "J";
    else if constexpr(T == return_type::float_)
        return "F";
    else if constexpr(T == return_type::double_)
        return "D";

    else if constexpr(T == return_type::bool_array_)
        return "[Z";
    else if constexpr(T == return_type::byte_array_)
        return "[B";
    else if constexpr(T == return_type::char_array_)
        return "[C";
    else if constexpr(T == return_type::short_array_)
        return "[S";
    else if constexpr(T == return_type::int_array_)
        return "[I";
    else if constexpr(T == return_type::long_array_)
        return "[J";
    else if constexpr(T == return_type::float_array_)
        return "[F";
    else if constexpr(T == return_type::double_array_)
        return "[D";

    else if constexpr(T == return_type::object_)
        return "L" + slashify(c) + ";";
    else if constexpr(T == return_type::object_array_)
        return "[L" + slashify(c) + ";";

    else if constexpr(T == return_type::void_)
        return "V";

    throw std::runtime_error("invalid type");
}

/* Exceptional types */

template<
    typename T,
    typename std::enable_if<std::is_same<T, void>::value>::type* = nullptr>
inline std::string to_str()
{
    return "V";
}

} // namespace arguments

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
        method.ret(arguments::to_str<T2>());
        return {std::move(method)};
    }

    /*!
     * \brief define an object return type
     * \param ret_type Java class name for return type
     * \return
     */
    jmethod<return_type::object_, Args...> ret(std::string const& ret_type)
    {
        method.ret(arguments::classify(ret_type));
        return {std::move(method)};
    }

    template<
        return_type T2,
        typename std::enable_if<T2 == return_type::object_array_>::type* =
            nullptr>
    /*!
     * \brief define an object array return type
     * \param type Java class name of returned type
     * \return
     */
    jmethod<return_type::object_, Args...> ret(std::string const& type)
    {
        method.ret(arguments::to_str<T2>(type));
        return {std::move(method)};
    }

    template<typename T2>
    /*!
     * \brief define argument with POD or POD array type
     * \return
     */
    jmethod<RType, Args..., T2> arg()
    {
        method.arg(arguments::to_str<T2>());
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
        method.arg(arguments::classify(type));
        return {std::move(method)};
    }

    /*!
     * \brief define class or POD argument type using jvalue
     * \param type
     * \return
     */
    jmethod<RType, Args..., ::jvalue> arg(std::string const& type)
    {
        method.arg(arguments::classify(type));
        return {std::move(method)};
    }

    template<
        typename T2,
        typename std::enable_if<
            is_jarray<T2>::value &&
            std::is_same<T2, jobjectArray>::value>::type* = nullptr>
    /*!
     * \brief define object array argument
     * \param type class name for elements of array
     * \return
     */
    jmethod<return_type::object_, Args...> arg(std::string const& type)
    {
        method.arg(arguments::to_str<T2>(type));
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

template<return_type T>
struct jfield
{
    jfield(java::field&& field)
        : field(std::move(field))
    {
    }

    template<return_type T2>
    /*!
     * \brief define as POD or POD array type
     * \return
     */
    jfield<T2> as()
    {
        field.signature = arguments::to_str<T2>();
        return {std::move(field)};
    }

    /*!
     * \brief define as Java class type
     * \param type
     * \return
     */
    jfield<return_type::object_> as(std::string const& type)
    {
        field.signature = type;
        field.signature = "L" + arguments::slashify(field.signature) + ";";
        return {std::move(field)};
    }

    template<
        return_type T2,
        typename std::enable_if<T2 == return_type::object_array_>::type* =
            nullptr>
    /*!
     * \brief define as object array
     * \param type class name for elements
     * \return
     */
    jfield<T2> as(std::string const& type)
    {
        field.signature = arguments::to_str<T2>(type);
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
    jclass(::jclass clazz, std::string const& class_name)
        : clazz(clazz)
        , class_name(class_name)
    {
    }

    jclass(std::string const& clazz)
        : jclass(GetJNI()->FindClass(clazz.c_str()), clazz)
    {
    }

    template<typename... Args>
    /*!
     * \brief Construct an object using a constructor
     * Constructors are always called "<init>"
     * \param method
     * \param args
     * \return
     */
    jobject construct(
        jmethod<return_type::void_, Args...> const& method, Args... args);

    /*!
     * \brief Object instantiation, wraps an existing object
     * \param instance
     * \return
     */
    jobject operator()(java::object instance);

    jobject operator()(::jobject instance);

    jobject operator()(java::value instance);

    template<return_type RType, typename... Args>
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

    template<return_type T>
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

    inline operator std::string() const
    {
        return class_name;
    }

    java::clazz clazz;
    std::string class_name;
};

struct jobject
{
    jobject(java::object const& object)
        : object(object)
    {
    }

    template<return_type RType, typename... Args>
    invocation::instance_call<RType, Args...> operator[](
        jmethod<RType, Args...> const& method)
    {
        auto methodId = GetJNI()->GetMethodID(
            *object.clazz, method.name(), method.signature());

        invocation::call::check_exception();

        return {java::method_reference{object, methodId}};
    }

    template<return_type T>
    class_props::instance_field<T> operator[](jfield<T> const& field)
    {
        auto fieldId = GetJNI()->GetFieldID(
            *object.clazz, field.name(), field.signature());

        invocation::call::check_exception();

        return {java::field_reference{object, fieldId}};
    }

    operator ::jvalue()
    {
        jvalue out;
        out.l = object;
        return out;
    }

    operator java::object()
    {
        return object;
    }

    java::object object;
};

template<typename... Args>
inline jobject jclass::construct(
    jmethod<return_type::void_, Args...> const& method, Args... args)
{
    auto constructor =
        GetJNI()->GetMethodID(clazz, method.name(), method.signature());

    invocation::call::check_exception();

    return jobject{java::object{
        {clazz}, *invocation::constructor_call({clazz, constructor})(args...)}};
}

inline jobject jclass::operator()(java::object instance)
{
    if(!instance)
        throw java_exception("null object");

    return jobject(java::object{clazz, instance.instance});
}

inline jobject jclass::operator()(::jobject instance)
{
    return (*this)(java::object({}, instance));
}

inline jobject jclass::operator()(java::value instance)
{
    if(!instance)
        throw java_exception("null object");

    return (*this)(java::object({}, instance->l));
}

} // namespace wrapping

inline wrapping::jclass get_class(java::clazz const& clazz)
{
    std::string class_name = *clazz.name;

    class_name = wrapping::arguments::slashify(class_name);

    return {GetJNI()->FindClass(class_name.c_str()), class_name};
}

namespace java {
namespace objects {

inline std::string get_class(java::object instance)
{
    auto Object = jnipp::get_class({"java.lang.Object"});
    auto Class  = jnipp::get_class({"java.lang.Class"});

    auto getClass = wrapping::jmethod<return_type::void_>({"getClass"})
                        .ret("java.lang.Class");
    auto getName = wrapping::jmethod<return_type::void_>({"getName"})
                       .ret("java.lang.String");

    auto objectInstance = Object(instance);
    auto classObject    = objectInstance[getClass]();
    auto className      = Class(classObject)[getName]();

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

FORCEDINLINE jnipp::wrapping::jmethod<jnipp::return_type::void_>
operator"" _jmethod(const char* name, size_t)
{
    return {jnipp::java::method{name}};
}

FORCEDINLINE jnipp::wrapping::jfield<jnipp::return_type::void_>
operator"" _jfield(const char* name, size_t)
{
    return {jnipp::java::field{name}};
}

} // namespace jnipp_operators

inline void jnipp::invocation::call::check_exception()
{
    if(GetJNI()->ExceptionCheck() == JNI_TRUE)
    {
        java::exception_clear_scope _;

        auto exception = java::object({}, GetJNI()->ExceptionOccurred());

        GetJNI()->ExceptionClear();

        auto exceptionType = jnipp::java::objects::get_class(
            java::object(java::clazz{}, exception));

        auto Throwable = get_class(java::clazz{"java.lang.Throwable"});
        auto getMessage =
            wrapping::jmethod<return_type::void_>(java::method("getMessage"))
                .ret("java.lang.String");

        auto message = java::type_unwrapper<std::string>(
            Throwable(exception)[getMessage]());

        throw java_exception(
            exceptionType + ": " + static_cast<std::string>(message));
    }
}
