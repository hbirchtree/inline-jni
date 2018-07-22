# Inline-JNI
JNI to C++ wrapper which aims to make JNI a little bit more usable

A header-only wrapper for JNI functionality, making it less painful to execute Java code from C++.

**This library heavily uses C++11 user-defined literals, the minimum GCC version to use this is 4.8**

# What is this used for?
Primarily, I have used it for better Android integration from C++. Getting system information or calling Android APIs that are not exposed through the NDK becomes much easier, and the code is much more minimal.

As an example, this is the code to fetch the name of the hardware board on Android:

    auto Build = "android.os.Build"_jclass;
    auto BOARD = "BOARD"_jfield.as("java.lang.String");
    auto boardName = build[BOARD];

    jnipp::java::type_unwrapper<std::string> boardNameWrap(boardName);

    std::string boardNameValue = static_cast<std::string>(boardNameWrap);

For calling a method, you can do as such:

    // Specify class, runtime-checked
    auto File = "java.io.File"_jclass;
    // Specify method and signature, runtime-checked
    auto createTempFile = "createTempFile"_jmethod
                                .ret("java.io.File")
                                .arg("java.lang.String")
                                .arg("java.lang.String");

    auto basename = jnipp::java::type_wrapper<std::string>(someString);
    auto extension = jnipp::java::type_wrapper<std::string>(someString);

    // Static method call
    // Arguments are passed as jvalue, compile-time checked
    auto fileValue = File[createTempFile](basename, extension);

    // Instanced call
    // First, instantiate class from value
    auto fileInstance = File(value.l);
    // Specify method and its signature (runtime-checked)
    auto getCanonicalPath = "getCanonicalPath"_jmethod.ret("java.lang.String");
    
    auto path = fileInstance[getCanonicalPath]();

    ...
    Use the value from path
    ...

The syntax is modelled to be close to Java. The API is not perfect, but simplifies some aspects of interacting with JNI from C++.

If a JVM exception had occurred in any of the calls above, a `jnipp::java_exception` would be triggered on the C++ side, allowing the exception to be handled without repeating the JNI checks (even though it adds overhead).

# How do I use this?

Most of this library is header-only, but, for obvious reasons, it needs access to the JNI environment in order to stay safe and simple to use.

The following function must be defined:

    namespace jnipp {
    
    JNIEnv* GetJNI()
    {
        ...
    }
    
    }
    
Without this, it would become considerably less fun to use, as you would have to guarantee that the JNI is current to the thread.

Also, on that note, **this library is not made to be thread-safe**.

For thread-safety, you must implement `GetJNI()` in a thread-safe way that acquires a JNI environment for the thread, for example through `thread_local`.

