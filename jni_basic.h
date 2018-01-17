#pragma once

#include "jni_structures.h"
#include "jni_helpers.h"

namespace JNIPP {

/* Some usual data types */
static char JavaStringType[] = "java.lang.String";

using JavaString = JavaClass<JavaStringType, std::string, JObject<std::string>>;

/*
 *
 * Value retrieveal stuffs
 *
 *
 */

template<typename T>
T JavaObjectAsValue(JObject<T> const*)
{
    return T();
}

template<>
FORCEDINLINE std::string JavaObjectAsValue(JObject<std::string> const* b)
{
    std::string out;

    if(!JNIPP::GetJNI())
        return out;

    auto chars = GetJNI()->GetStringUTFChars((jstring)b->obj_ref, 0);
    if(chars)
        out = chars;
    GetJNI()->ReleaseStringUTFChars((jstring)b->obj_ref, chars);
    return out;
}

template<typename RType>
FORCEDINLINE JObject<RType> ObjectFieldToNative(JObjectField<RType>&& field)
{
    return RType();
}

template<>
FORCEDINLINE JObject<std::string> ObjectFieldToNative(JObjectField<std::string>&& field)
{
    if(!JNIPP::GetJNI())
        return {};

    auto fId = field;

    fId.getStaticFieldID(fId.fieldType.c_str());

    return {fId.c_class,
                GetJNI()->GetStaticObjectField(fId.c_class,
                                               fId.c_field),
                fId.type};
}

template<typename RType>
FORCEDINLINE RType ValueFieldToNative(JObjectField<RType>&& field)
{
    return RType();
}

template<>
FORCEDINLINE long ValueFieldToNative(JObjectField<long>&& field)
{
    if(!JNIPP::GetJNI())
        return -1;

    field.getStaticFieldID(field.fieldType.c_str());

    return GetJNI()->GetStaticLongField(field.c_class,
                                        field.c_field);
}

template<>
FORCEDINLINE int ValueFieldToNative(JObjectField<int>&& field)
{
    if(!JNIPP::GetJNI())
        return -1;

    field.getStaticFieldID(field.fieldType.c_str());

    return GetJNI()->GetStaticIntField(field.c_class,
                                        field.c_field);
}

/*
 *
 * Field conversions, ensuring correct values are retrieved
 *
 *
 */

template<typename T,
         typename std::enable_if<!std::is_pod<T>::value, bool>::type*,
         typename std::enable_if<!std::is_same<T, std::string>::value, bool>::type*>
JObjectField<T> FieldAs(JObjectField<void>&&)
{
    return {};
}

template<typename T>
FORCEDINLINE JObjectField<T> FieldAs(JObjectField<void>&& f)
{
    f.fieldType = TranslateJavaArg<T>();
    return static_cast<JObjectField<T>>(f);
}

/*
 *
 * Related to methods and fetching handles to them
 *
 *
 */

template<typename RType>
FORCEDINLINE void ExtractReturnType(std::string* returnType)
{
    (*returnType) = "V";
}

template<>
FORCEDINLINE void ExtractReturnType<void>(std::string* returnType)
{
    (*returnType) = "V";
}

//template<typename Arg>
//FORCEDINLINE void ExtractArgs(JMethod<void>* method)
//{
//    method->argType.push_back(JavaArgTypeToSig<Arg>());
//}

/*
 *
 * Calling into JNI methods
 *
 *
 */

template<typename T>
FORCEDINLINE jvalue ObjectMethodCall(
        jobject obj, jmethodID method, jvalue* args)
{
    jvalue out;
    if(!JNIPP::GetJNI())
        return out;

    out.l = GetJNI()->CallObjectMethodA(obj, method, args);
    return out;
}

template<>
FORCEDINLINE jvalue ObjectMethodCall<void>(
        jobject obj, jmethodID method, jvalue* args)
{
    if(!JNIPP::GetJNI())
        return {};
    GetJNI()->CallVoidMethodA(obj, method, args);
    return {};
}

}
