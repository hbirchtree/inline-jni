#pragma once

#ifndef FORCEDINLINE
#define FORCEDINLINE inline
#endif

#include <jni.h>
#include <string>

namespace jnipp {

template<typename T>
struct is_jarray
{
    static constexpr bool value =
        std::is_base_of<_jarray, typename std::remove_pointer<T>::type>::value;
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

extern JNIEnv* GetJNI();

template<typename T>
using optional = std::optional<T>;

namespace java {

struct clazz
{
    clazz(std::string const& name)
        : name(name)
    {
    }

    clazz(::jclass ref)
        : class_ref(ref)
    {
    }

    clazz()
    {
    }

    operator const char*() const
    {
        return name->c_str();
    }

    operator ::jclass() const
    {
        return *class_ref;
    }

    optional<std::string> name;
    optional<::jclass>    class_ref;
};

struct method
{
    method(std::string const& name)
        : name(name)
        , signature("()V")
    {
    }

    method(
        ::jmethodID           method_id,
        optional<std::string> return_class = std::nullopt)
        : method_id(method_id)
        , return_class(return_class)
    {
    }

    inline std::string returnType()
    {
        auto returnSplit = signature.find(")") + 1;
        return signature.substr(returnSplit, std::string::npos);
    }

    inline std::string argList()
    {
        auto returnSplit = signature.find("(");
        auto endSplit    = signature.find(")");
        return signature.substr(returnSplit + 1, endSplit - returnSplit - 1);
    }

    void ret(std::string const& retType)
    {
        auto returnSplit = signature.find(")");
        signature        = signature.substr(0, returnSplit + 1) + retType;
    }

    void arg(std::string const& argType)
    {
        auto endSplit = signature.find(")");

        auto begin = signature.substr(0, endSplit);
        auto end   = signature.substr(endSplit);

        signature = begin + argType + end;
    }

    ::jmethodID operator*() const
    {
        return *method_id;
    }

    std::string                name;
    std::string                signature;
    std::optional<std::string> return_class;
    optional<::jmethodID>      method_id;
};

struct field
{
    field(std::string const& name)
        : name(name)
    {
    }

    field(::jfieldID field_id, optional<std::string> field_class = std::nullopt)
        : field_id(field_id)
        , signature(field_class.value_or(std::string()))
    {
    }

    field& withType(std::string const& sig)
    {
        signature = sig;
        return *this;
    }

    ::jfieldID operator*() const
    {
        return *field_id;
    }

    std::string          name;
    std::string          signature;
    optional<::jfieldID> field_id;
};

struct array
{
    ::jarray    instance;
    ::jclass    value_class;
    std::string value_type;

    operator ::jarray() const
    {
        return instance;
    }

    jlong length() const
    {
        return GetJNI()->GetArrayLength(instance);
    }

    template<typename T>
    static array of(T* arrayType)
    {
        return array{reinterpret_cast<::jarray>(arrayType)};
    }
};

struct object
{
    object(java::clazz clazz, ::jobject instance)
        : clazz(*clazz.class_ref)
        , instance(instance)
    {
    }

    object()
    {
    }

    optional<::jclass> clazz;
    ::jobject          instance;

    operator ::jobject() const
    {
        return instance;
    }

    optional<java::array> array() const
    {
        if(!instance)
            return {};
        return java::array{reinterpret_cast<::jarray>(instance)};
    }

    inline operator bool() const
    {
        return instance;
    }
};

using value = optional<::jvalue>;

template<typename T>
inline value make_value(T val)
{
    ::jvalue out;
    if constexpr(std::is_same_v<T, jboolean>)
        out.z = val;
    else if constexpr(std::is_same_v<T, jbyte>)
        out.b = val;
    else if constexpr(std::is_same_v<T, jchar>)
        out.b = val;
    else if constexpr(std::is_same_v<T, jshort>)
        out.s = val;
    else if constexpr(std::is_same_v<T, jint>)
        out.i = val;
    else if constexpr(std::is_same_v<T, jlong>)
        out.j = val;
    else if constexpr(std::is_same_v<T, jfloat>)
        out.f = val;
    else if constexpr(std::is_same_v<T, jdouble>)
        out.d = val;
    else if constexpr(std::is_same_v<T, jobject>)
        out.l = val;
    else
        return {};

    return out;
}

struct method_reference
{
    java::object instance;
    java::method method;
};

struct static_method_reference
{
    java::clazz  clazz;
    java::method method;
};

struct field_reference
{
    java::object instance;
    java::field  field;
};

struct static_field_reference
{
    java::clazz clazz;
    java::field field;
};

} // namespace java

} // namespace jnipp
