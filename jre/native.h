
#ifndef JAVA_NATIVE_H
#define JAVA_NATIVE_H

#include <cstddef>

struct java_native_method_t
{
    const char *class_name;
    const char *name;
    const char *sig;
    void *func;
};

extern const struct java_native_method_t java_native_methods[];

#endif // JAVA_NATIVE_H

