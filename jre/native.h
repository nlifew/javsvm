
#ifndef JAVA_NATIVE_H
#define JAVA_NATIVE_H

#include <cstddef>
#include <type_traits>

struct java_native_method_t
{
//    const char *class_name;
//    const char *name;
//    const char *sig;
    const char *jni_name;
    void *func;
};

extern const struct java_native_method_t java_native_methods[];

#endif // JAVA_NATIVE_H

