#pragma once

#ifndef FORCEDINLINE
#define FORCEDINLINE inline
#endif

#include <jni.h>
#include <string>

namespace jnipp {

extern JNIEnv* GetJNI();

template<typename T>
using optional = std::optional<T>;

namespace java {

struct method
{
    method(std::string const& name) : name(name), signature("()V")
    {
    }
    method(::jmethodID method_id) : method_id(method_id)
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

        signature = signature.substr(0, returnSplit + 1) + retType;
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

    std::string name;
    std::string signature;
    optional<::jmethodID> method_id;
};

struct field
{
    field(std::string const& name) : name(name)
    {
    }
    field(::jfieldID field_id) : field_id(field_id)
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

    std::string name;
    std::string signature;
    optional<::jfieldID> field_id;
};

struct clazz
{
    clazz(std::string const& name) : name(name)
    {
    }
    clazz(::jclass ref) : class_ref(ref)
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

struct array
{
    ::jarray instance;

    jlong length() const
    {
        return GetJNI()->GetArrayLength(instance);
    }
};

struct object
{
    object(java::clazz clazz, ::jobject instance) :
        clazz(*clazz.class_ref), instance(instance)
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

template<typename T>
struct type_wrapper
{
    type_wrapper(T)
    {
    }

    operator java::value()
    {
        return java::value();
    }
};

template<typename T>
struct type_unwrapper
{
    type_unwrapper(java::value value) : value(value)
    {
    }

    operator T() const
    {
        return T();
    }

    java::value value;
};

} // namespace java

} // namespace jnipp
