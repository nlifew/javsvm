/**
 * auto generated by javsvm-javah, don't modify it.
 */

#ifndef JAVA_SUN_REFLECT_REFLECTION_H
#define JAVA_SUN_REFLECT_REFLECTION_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * class: sun/reflect/Reflection
 * method: getCallerClass
 * sig: ()Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL
Java_sun_reflect_Reflection_getCallerClass__
    (JNIEnv *env, jclass klass);

/**
 * class: sun/reflect/Reflection
 * method: getCallerClass
 * sig: (I)Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL
Java_sun_reflect_Reflection_getCallerClass__I
    (JNIEnv *env, jclass klass, jint);

/**
 * class: sun/reflect/Reflection
 * method: getClassAccessFlags
 * sig: (Ljava/lang/Class;)I
 */
JNIEXPORT jint JNICALL
Java_sun_reflect_Reflection_getClassAccessFlags
    (JNIEnv *env, jclass klass, jclass);



#ifdef __cplusplus
}
#endif
#endif // JAVA_SUN_REFLECT_REFLECTION_H
