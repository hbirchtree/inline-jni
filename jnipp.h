#pragma once

#include "jni_structures.h"
#include "jni_basic.h"
#include "jni_operators.h"

namespace JNIPP {

template<typename OT>
struct JObject;
struct JClass;

template<typename T>
void JObjectField<T>::getStaticFieldID(const char *type)
{
    if(!JNIPP::GetJNI())
        return;
    c_field = GetJNI()->GetStaticFieldID(c_class, fieldName.c_str(), type);
}

template<typename T>
void JObjectField<T>::getFieldID(const char *type)
{
    if(!JNIPP::GetJNI())
        return;
    c_field = GetJNI()->GetFieldID(c_class, fieldName.c_str(), type);
}

template<typename OT>
OT JObject<OT>::value()
{
    return JavaObjectAsValue(this);
}

/*
 *
 * Class operators, such that writing the code becomes short while functional
 *
 */

FORCEDINLINE JMethod<void> JClass::operator[](JMethod<void>&& method)
{
    method.c_class = c_class;
    return method;
}

}

using namespace JNIPP;
