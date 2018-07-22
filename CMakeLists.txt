project ( inline-jni )
cmake_minimum_required ( VERSION 3.0 )

find_package ( JNI REQUIRED )

add_executable ( JNIExample examples/main.cpp )

target_link_libraries ( JNIExample PUBLIC
    ${JNI_LIBRARIES}
    )

target_include_directories ( JNIExample PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${JNI_INCLUDE_DIRS}
    ${JAVA_INCLUDE_PATH}
    )

set_property ( TARGET JNIExample PROPERTY CXX_STANDARD 11 )
