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
    JavaVM* jvm = nullptr;

    JavaVMInitArgs vm_args;
    JavaVMOption option;
    option.optionString = "-Djava.class.path=/usr/lib/java";
    vm_args.version = JNI_VERSION_1_6;
    vm_args.nOptions = 1;
    vm_args.options = &option;
    vm_args.ignoreUnrecognized = false;
    JNI_CreateJavaVM(&jvm, (void**)&globalEnv, &vm_args);

    auto File = "java.io.File"_jclass;
    auto createTempFile = "createTempFile"_jmethod
        .ret("java.io.File")
        .arg("java.lang.String")
        .arg("java.lang.String");

    auto arg1 = jnipp::java::type_wrapper<std::string>("test");
    auto arg2 = jnipp::java::type_wrapper<std::string>(".png");

    auto tempFile = File[createTempFile](arg1, arg2);

    printf("%p\n", tempFile.l);

    jvm->DestroyJavaVM();
}
