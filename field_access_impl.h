#pragma once

#include "class_constructor.h"
#include "class_wrapper.h"
#include "field_access.h"

namespace jnipp::field_access {

template<return_type T>
inline auto instance_field<T>::operator*() const
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
        return wrapping::jobject(java::object{
            {},
            GetJNI()->GetObjectField(field.instance, *field.field),
        });
    else
        return java::value();
}

template<return_type T>
inline auto static_field<T>::operator*() const
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
        return wrapping::jobject(java::object{
            {},
            GetJNI()->GetStaticObjectField(field.clazz, *field.field),
        });
    else
        return java::value();
}

} // namespace jnipp::field_access
