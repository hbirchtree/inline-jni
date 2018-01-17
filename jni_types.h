#pragma once

#ifndef FORCEDINLINE
#define FORCEDINLINE inline
#endif

#include <jni.h>

namespace JNIPP {

enum JType
{
    JNI_GenericObjectType,

    JNI_StringType,
    JNI_IntegerType,
};

}
