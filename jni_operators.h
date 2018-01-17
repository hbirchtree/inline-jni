#pragma once

#include <algorithm>

#include "jni_structures.h"
#include "jni_types.h"

FORCEDINLINE JNIPP::JClass operator "" _jclass(const char* className, size_t)
{
    std::string classNameStd = className;

    std::replace(classNameStd.begin(), classNameStd.end(),
                 '.', '/');

    if(!JNIPP::GetJNI())
        return {};

    return {JNIPP::GetJNI()->FindClass(classNameStd.c_str())};
}

FORCEDINLINE JNIPP::JObjectField<void> operator "" _jfield(const char* fieldName, size_t)
{
    return {fieldName, {}, NULL, NULL, JNIPP::JNI_GenericObjectType};
}

FORCEDINLINE JNIPP::JType operator "" _jtype(const char* s, size_t)
{
    if(std::string(s) == "java.lang.String")
        return JNIPP::JNI_StringType;

    return JNIPP::JNI_GenericObjectType;
}

FORCEDINLINE JNIPP::JMethod<void> operator "" _jmethod(const char* s, size_t)
{
    /* At this point we only know the method name, no class association */
    /* We still need the class and signature of the function */
    return {s, NULL, NULL};
}
