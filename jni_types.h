#pragma once

#ifndef FORCEDINLINE
#define FORCEDINLINE inline
#endif

#include <jni.h>
#include <string>

namespace jnipp {

namespace java {

struct method
{
    method(std::string const& name) :
        name(name), signature("()V")
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

    std::string name;
    std::string signature;
};

struct field
{
    field(std::string const& name) : name(name)
    {
    }

    field& withType(std::string const& sig)
    {
        signature = sig;
        return *this;
    }

    std::string name;
    std::string signature;
};

struct clazz
{
    clazz(std::string const& name) : name(name)
    {
    }

    operator const char*()
    {
        return name.c_str();
    }

    std::string name;
};

struct object
{
    jclass  clazz;
    jobject instance;
};

struct method_reference
{
    jobject   instance;
    jmethodID methodId;
};

struct static_method_reference
{
    jclass    clazz;
    jmethodID methodId;
};

struct field_reference
{
    jobject  instance;
    jfieldID fieldId;
};

struct static_field_reference
{
    jclass   clazz;
    jfieldID fieldId;
};

template<typename T>
struct type_wrapper
{
    type_wrapper(T)
    {
    }

    operator jvalue()
    {
        return jvalue();
    }
};

template<typename T>
struct type_unwrapper
{
    type_unwrapper(jvalue value):
        value(value)
    {
    }

    operator T() const
    {
        return T();
    }

    jvalue value;
};

} // namespace java

} // namespace jnipp
