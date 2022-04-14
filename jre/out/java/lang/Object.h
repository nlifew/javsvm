/**
 * auto generated by javsvm-javah, don't modify it.
 */

#ifndef JAVA_JAVA_LANG_OBJECT_H
#define JAVA_JAVA_LANG_OBJECT_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * class: java/lang/Object
 * method: clone
 * sig: ()Ljava/lang/Object;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_Object_clone
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Object
 * method: getClass
 * sig: ()Ljava/lang/Class;
 */
JNIEXPORT jclass JNICALL
Java_java_lang_Object_getClass
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Object
 * method: notify
 * sig: ()V
 */
JNIEXPORT void JNICALL
Java_java_lang_Object_notify
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Object
 * method: notifyAll
 * sig: ()V
 */
JNIEXPORT void JNICALL
Java_java_lang_Object_notifyAll
    (JNIEnv *env, jobject self);

/**
 * class: java/lang/Object
 * method: wait
 * sig: (J)V
 */
JNIEXPORT void JNICALL
Java_java_lang_Object_wait
    (JNIEnv *env, jobject self, jlong);



#ifdef __cplusplus
}
#endif
#endif // JAVA_JAVA_LANG_OBJECT_H
