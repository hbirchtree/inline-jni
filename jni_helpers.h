#pragma once

#include <string>
#include <algorithm>
#include "jni_types.h"

namespace JNIPP {

/* Function that retrieves the JNI interface */
extern JNIEnv* GetJNI();

template<typename T>
FORCEDINLINE const char* JavaArgTypeToSig()
{
//    static_assert(false, "Unimplemented type conversion");
    return "_";
}

template<>
FORCEDINLINE const char* JavaArgTypeToSig<bool>()
{
    return "Z";
}

template<>
FORCEDINLINE const char* JavaArgTypeToSig<char>()
{
    return "B";
}

template<>
FORCEDINLINE const char* JavaArgTypeToSig<short>()
{
    return "S";
}

template<>
FORCEDINLINE const char* JavaArgTypeToSig<int>()
{
    return "I";
}

template<>
FORCEDINLINE const char* JavaArgTypeToSig<long>()
{
    return "J";
}

template<>
FORCEDINLINE const char* JavaArgTypeToSig<float>()
{
    return "F";
}

template<>
FORCEDINLINE const char* JavaArgTypeToSig<double>()
{
    return "D";
}

template<>
FORCEDINLINE const char* JavaArgTypeToSig<void>()
{
    return "V";
}

template<char const* Name, typename NativeType, typename WrapperType>
struct JavaClass
{
    static constexpr const char* name = Name;
    typedef NativeType target;
    typedef WrapperType wrapper;
};

template<typename T>
FORCEDINLINE std::string JavaArgClassToSig()
{
    std::string className = T::name;

    std::replace(className.begin(), className.end(),
                 '.', '/');

    className = "L" + className + ";";

    return className;
}

template<typename T,
         typename std::enable_if<std::is_class<T>::value, bool>::type* = nullptr>
FORCEDINLINE std::string TranslateJavaArg()
{
    return JavaArgClassToSig<T>();
}

template<typename T,
         typename std::enable_if<!std::is_class<T>::value, bool>::type* = nullptr>
FORCEDINLINE const char* TranslateJavaArg()
{
    return JavaArgTypeToSig<T>();
}

//template<>
//FORCEDINLINE const char* JavaArgTypeToSig<std::string>()
//{
//    return "Ljava/lang/String;";
//}

template<typename Arg, typename... Args>
constexpr bool IsEmptyArgumentStack(Arg, Args...)
{
    return false;
}
template<typename Arg,
         typename std::enable_if<
             std::is_same<
                 Arg, std::nullptr_t>::value,
             bool>::type* = nullptr>
constexpr bool IsEmptyArgumentStack(Arg)
{
    return true;
}
template<typename Arg,
         typename std::enable_if<
             !std::is_same<
                 Arg, std::nullptr_t>::value,
             bool>::type* = nullptr>
constexpr bool IsEmptyArgumentStack(Arg)
{
    return false;
}

}
