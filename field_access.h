#pragma once

#include "jni_types.h"
#include "type_signatures.h"

namespace jnipp::field_access {

template<return_type T>
struct instance_field
{
    auto operator*() const;

    java::field_reference field;
};

template<return_type T>
struct static_field
{
    auto operator*() const;

    java::static_field_reference field;
};

} // namespace jnipp::field_access

namespace jnipp::wrapping {

template<return_type T>
struct jfield
{
    jfield(java::field&& field)
        : field(std::move(field))
    {
    }

    template<return_type T2>
    /*!
     * \brief define as POD or POD array type
     * \return
     */
    jfield<T2> as()
    {
        field.signature = type_signature::to_str<T2>();
        return {std::move(field)};
    }

    /*!
     * \brief define as Java class type
     * \param type
     * \return
     */
    jfield<return_type::object_> as(std::string const& type)
    {
        field.signature = type;
        field.signature = "L" + type_signature::slashify(field.signature) + ";";
        return {std::move(field)};
    }

    template<return_type T2>
    requires(T2 == return_type::object_array_)
    /*!
     * \brief define as object array
     * \param type class name for elements
     * \return
     */
    jfield<T2> as(std::string const& type)
    {
        field.signature = type_signature::to_str<T2>(type);
        return {std::move(field)};
    }

    const char* name() const
    {
        return field.name.c_str();
    }

    const char* signature() const
    {
        return field.signature.c_str();
    }

    java::field field;
};

} // namespace jnipp::wrapping
