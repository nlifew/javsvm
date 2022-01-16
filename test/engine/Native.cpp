
#include "Native.h"

#include <cstdint>

#include "object/jmethod.h"
#include "jni/jni_utils.h"

extern "C" JNIEXPORT jint JNICALL
Java_Native_nAdd__II
        (JNIEnv *env, jclass clazz, jint a, jint b)
{
//    javsvm::jclass *_class = to_class(clazz);

    jmethodID add = env->GetMethodID(clazz, "add", "(II)I");
    jmethodID _init_ = env->GetMethodID(clazz, "<init>", "()V");

    jobject obj = env->NewObject(clazz, _init_);

    return env->CallIntMethod(obj, add, a, b);
}


extern "C" JNIEXPORT jdouble JNICALL Java_Native_nAdd__ZBCSIJFDZBCSIJFD
        (JNIEnv *env, jclass clazz,
         jboolean z, jbyte b, jchar c, jshort s, jint i, jlong j, jfloat f, jdouble d,
         jboolean z1, jbyte b1, jchar c1, jshort s1, jint i1, jlong j1, jfloat f1, jdouble d1)
{
    double hash = 0;
    hash = 31 * hash + z;
    hash = 31 * hash + b;
    hash = 31 * hash + c;
    hash = 31 * hash + s;
    hash = 31 * hash + i;
    hash = 31 * hash + j;
    hash = 31 * hash + f;
    hash = 31 * hash + d;

    hash = 31 * hash + z1;
    hash = 31 * hash + b1;
    hash = 31 * hash + c1;
    hash = 31 * hash + s1;
    hash = 31 * hash + i1;
    hash = 31 * hash + j1;
    hash = 31 * hash + f1;
    hash = 31 * hash + d1;

    return hash + 1.0;
}
