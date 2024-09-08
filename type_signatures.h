#pragma once

#include "jni_types.h"

namespace jnipp::type_signature {

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

}
