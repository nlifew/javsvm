
#include "jni/jni_utils.h"
#include "sun/reflect/Reflection.h"

/**
 * class: sun/reflect/Reflection
 * method: getCallerClass
 * sig: ()Ljava/lang/Class;
 */
extern "C" JNIEXPORT jclass JNICALL
Java_sun_reflect_Reflection_getCallerClass__
    (JNIEnv *, jclass)
{
    safety_area_guard guard;

    int depth = 2;
    for (auto f = javsvm::jvm::get().env().stack.top(); f; f = f->next) {
        if (depth-- == 0)
            return to_object<jclass>(f->method->clazz->object.get());
    }
    return nullptr;
}

/**
 * class: sun/reflect/Reflection
 * method: getCallerClass
 * sig: (I)Ljava/lang/Class;
 */
extern "C" JNIEXPORT jclass JNICALL
Java_sun_reflect_Reflection_getCallerClass__I
    (JNIEnv *, jclass, jint depth)
{
    safety_area_guard guard;
    for (auto f = javsvm::jvm::get().env().stack.top(); f; f = f->next) {
        if (depth-- == 0)
            return to_object<jclass>(f->method->clazz->object.get());
    }
    return nullptr;
}

/**
 * class: sun/reflect/Reflection
 * method: getClassAccessFlags
 * sig: (Ljava/lang/Class;)I
 */
extern "C" JNIEXPORT jint JNICALL
Java_sun_reflect_Reflection_getClassAccessFlags
    (JNIEnv *, jclass, jclass klass)
{
    safety_area_guard guard;
    return (jint) to_class(klass)->access_flag;
}

