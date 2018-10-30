#include <jnipp.h>

static JNIEnv* globalEnv;

namespace jnipp {

JNIEnv* GetJNI()
{
    return globalEnv;
}

}

int main()
{
    /* Very important before using _jclass and _jmethod.
     * This imports the operator"" functions */
    using namespace jnipp_operators;

    /* Standard JNI initialization, nothing special here */
    JavaVM* jvm = nullptr;

    JavaVMInitArgs vm_args;
    JavaVMOption option;
    option.optionString = const_cast<char*>("-Djava.class.path=/usr/lib/java");
    vm_args.version = JNI_VERSION_1_6;
    vm_args.nOptions = 1;
    vm_args.options = &option;
    vm_args.ignoreUnrecognized = false;
    JNI_CreateJavaVM(&jvm, (void**)&globalEnv, &vm_args);

    /* We define our prototype class and method.
     * Note how fully qualified class names are used. */
    auto File = "java.io.File"_jclass;
    auto createTempFile = "createTempFile"_jmethod
        .ret("java.io.File")
        .arg("java.lang.String")
        .arg("java.lang.String");

    /* Wrapper objects for "translating" C++ types
     * The pre-defined wrappers support POD types and std::string. */
    auto arg1 = jnipp::java::type_wrapper<std::string>("test");
    auto arg2 = jnipp::java::type_wrapper<std::string>(".png");

    /* Call the function... */
    auto tempFile = File[createTempFile](arg1, arg2);

    /* ... and get the object's handle here */
    printf("File.createTempFile(...) -> %p\n", tempFile.l);

    {
        /* We can also extract a string if we want to, with some extra work */
        auto getCanonicalPath = "getCanonicalPath"_jmethod
            .ret("java.lang.String");
       
        /* Differently to using a static function, we "instantiate" a proxy 
         * object using the object we got earlier. The class is called with the
         * handle. All operations are performed on the object, both fields
         * and methods. */
        auto canonicalPath = File(tempFile)[getCanonicalPath]();

        /* Type unwrappers take the jobject return value and allows static_cast<>
         *  to its target type.
         * This allows the C++ type system to work on them. */
        std::string canonicalPathUnwrapped = 
            jnipp::java::type_unwrapper<std::string>(canonicalPath);

        /* The final type can be used here. */
        printf("tempFile.getCanonicalPath() -> %s\n",
                canonicalPathUnwrapped.c_str());
    }

    for(unsigned int i = 0; i<1024 * 1024 ; i++)
    {
        /* Using a POD-type instead of a more complex type.
         * Note that we use the JNI type to refer to the type we want. */
        auto getUsableSpace = "getUsableSpace"_jmethod
            .ret<jlong>();

        /* jlong is usable by C++ */
        jlong usableSpace = 
            jnipp::java::type_unwrapper<jlong>(File(tempFile)[getUsableSpace]());

        printf("tempFile.getUsableSpace() -> %li\n", usableSpace);
    }

    jvm->DestroyJavaVM();
}
