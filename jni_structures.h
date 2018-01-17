#pragma once

#include <string>
#include <stdint.h>

#include <stddef.h>

#include "jni_helpers.h"

namespace JNIPP {

template<typename T>
struct JObjectField {
    std::string fieldName;
    std::string fieldType;
    jclass c_class;
    jfieldID c_field;

    JType type;

    void getStaticFieldID(const char* type);
    void getFieldID(const char* type);

    template<typename T2>
    operator JObjectField<T2>()
    {
        return {fieldName, fieldType, c_class, c_field, type};
    }
};

template<typename T,
         typename std::enable_if<!std::is_pod<T>::value, bool>::type* = nullptr,
         typename std::enable_if<!std::is_same<T, std::string>::value, bool>::type* = nullptr>
JObjectField<T> FieldAs(JObjectField<void>&& f);

template<typename RType>
struct JMethod {
    std::string methodName;
    jclass c_class;
    jmethodID c_method;

    std::string returnType;
    std::string argType;

    template<typename RType2>
    operator JMethod<RType2>()
    {
        return {methodName, c_class, c_method, returnType, argType};
    }

    template<typename Arg>
    JMethod<RType>& with()
    {
        argType += TranslateJavaArg<Arg>();
        return *this;
    }

    template<typename RType2>
    JMethod<RType2> returns()
    {
        returnType = TranslateJavaArg<RType2>();
        return static_cast<JMethod<RType2>>(*this);
    }
};

template<typename RType>
jvalue ObjectMethodCall(jobject obj, jmethodID method, jvalue* args);

template<typename OT>
struct JObject;

template<typename RType>
struct JObjectMethodInstance {
    std::string methodName;
    jclass c_class;
    jobject obj_ref;
    jmethodID c_method;

    jvalue operator()(std::vector<jvalue>&& arguments)
    {
        return ObjectMethodCall<RType>(obj_ref, c_method,
                                       arguments.data());
    }
};

template<typename OT>
struct JObject {
    jclass c_obj;
    jobject obj_ref;

    JType obj_type;

    template<typename T>
    jfieldID fieldId(JType* jt, JObjectField<T> const& field)
    {
        auto fId = field.getFieldID(TranslateJavaArg<T>());

        return fId;
    }

    template<typename RType>
    JObjectMethodInstance<RType> operator[](
            JMethod<RType>&& method)
    {
        if(method.c_class == NULL)
            method.c_class = c_obj;

        std::string methodSignature = "(" + method.argType + ")"
                + method.returnType;

        if(!JNIPP::GetJNI())
            return {};

        method.c_method = GetJNI()->GetMethodID(method.c_class,
                                                method.methodName.c_str(),
                                                methodSignature.c_str());

        if(c_obj == method.c_class)
            return {method.methodName, method.c_class, obj_ref, method.c_method};
        else
            return {};
    }

    OT value();
};

template<typename RType>
JObject<RType> ObjectFieldToNative(JObjectField<RType>&& field);

template<typename RType>
RType ValueFieldToNative(JObjectField<RType>&& field);

struct JClass {
    jclass c_class;

    FORCEDINLINE JObject<int> operator()(jobject instance)
    {
        return {c_class, instance, JNI_GenericObjectType};
    }

    template<typename RType,
             typename std::enable_if<std::is_class<RType>::value, bool>::type* = nullptr>
    /* For object  types */
    typename RType::wrapper operator[](JObjectField<RType>&& field)
    {
        field.c_class = c_class;
        return ObjectFieldToNative<typename RType::target>(std::move(field));
    }

    template<typename RType,
             typename std::enable_if<!std::is_class<RType>::value, bool>::type* = nullptr>
    /* For object  types */
    RType operator[](JObjectField<RType>&& field)
    {
        field.c_class = c_class;
        return ValueFieldToNative<RType>(std::move(field));
    }

    JMethod<void> operator[](JMethod<void>&& field);
};

}
