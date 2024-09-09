#pragma once

#include "field_access.h"
#include "jni_types.h"
#include "method_calls.h"
#include "type_signatures.h"

namespace jnipp::wrapping {

struct jobject;

struct jclass
{
    jclass(::jclass clazz, std::string const& class_name)
        : clazz(clazz)
        , class_name(class_name)
    {
        this->clazz.name = type_signature::slashify(class_name);
    }

    jclass(std::string const& clazz)
        : jclass(
              GetJNI()->FindClass(type_signature::slashify(clazz).c_str()),
              clazz)
    {
    }

    template<typename... Args>
    /*!
     * \brief Construct an object using a constructor
     * Constructors are always called "<init>"
     * \param method
     * \param args
     * \return
     */
    jobject construct(
        jmethod<return_type::void_, Args...> const& method, Args... args);

    /*!
     * \brief Object instantiation, wraps an existing object
     * \param instance
     * \return
     */
    jobject operator()(java::object instance);

    jobject operator()(::jobject instance);

    jobject operator()(java::value instance);

    template<return_type RType, typename... Args>
    /*!
     * \brief Static method calls
     * \param method
     * \return
     */
    invocation::static_call<RType, Args...> operator[](
        jmethod<RType, Args...> const& method)
    {
        auto methodId = GetJNI()->GetStaticMethodID(
            clazz, method.name(), method.signature());

        invocation::call::check_exception();

        return {
            java::static_method_reference({
                clazz,
                java::method(methodId, method.method.return_class),
            }),
        };
    }

    template<return_type T>
    /*!
     * \brief Static fields
     * \param field
     * \return
     */
    field_access::static_field<T> operator[](jfield<T> const& field)
    {
        auto fieldId =
            GetJNI()->GetStaticFieldID(clazz, field.name(), field.signature());

        invocation::call::check_exception();

        return {
            java::static_field_reference({
                clazz,
                java::field(fieldId, field.field.signature),
            }),
        };
    }

    inline operator std::string() const
    {
        return class_name;
    }

    java::clazz clazz;
    std::string class_name;
};

struct jobject
{
    jobject(java::object const& object)
        : object(object)
    {
        // this->object.instance = GetJNI()->NewGlobalRef(object.instance);
    }

    ~jobject()
    {
        // GetJNI()->DeleteGlobalRef(object.instance);
    }

    inline jobject cast(jclass const& clazz) const;

    template<return_type RType, typename... Args>
    invocation::instance_call<RType, Args...> operator[](
        jmethod<RType, Args...> const& method)
    {
        auto methodId = GetJNI()->GetMethodID(
            *object.clazz, method.name(), method.signature());

        invocation::call::check_exception();

        return {java::method_reference{
            object,
            java::method(methodId, method.method.return_class),
        }};
    }

    template<return_type T>
    field_access::instance_field<T> operator[](jfield<T> const& field)
    {
        auto fieldId = GetJNI()->GetFieldID(
            *object.clazz, field.name(), field.signature());

        invocation::call::check_exception();

        return {java::field_reference{
            object,
            java::field(fieldId, field.field.signature),
        }};
    }

    inline operator bool() const
    {
        return static_cast<bool>(object);
    }

    inline operator ::jvalue() const
    {
        return ::jvalue{
            .l = object,
        };
    }

    inline operator java::object() const
    {
        return object;
    }

    java::object object;
};

template<typename... Args>
inline jobject jclass::construct(
    jmethod<return_type::void_, Args...> const& method, Args... args)
{
    auto constructor =
        GetJNI()->GetMethodID(clazz, method.name(), method.signature());

    invocation::call::check_exception();

    return jobject{java::object{
        {clazz},
        *invocation::constructor_call({clazz, constructor})(args...),
    }};
}

inline jobject jclass::operator()(java::object instance)
{
    return jobject(java::object{clazz, instance.instance});
}

inline jobject jclass::operator()(::jobject instance)
{
    return (*this)(java::object({}, instance));
}

inline jobject jclass::operator()(java::value instance)
{
    return (*this)(java::object({}, instance->l));
}

} // namespace jnipp::wrapping

namespace jnipp::java::objects {

inline bool is_null(wrapping::jobject const& obj)
{
    return obj.object.instance;
}

} // namespace jnipp::java::objects
