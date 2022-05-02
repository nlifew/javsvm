/**
 * auto generated by javsvm-javah, don't modify it.
 */

#ifndef JAVA_JAVA_LANG_SYSTEM_H
#define JAVA_JAVA_LANG_SYSTEM_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * class: java/lang/System
 * method: arraycopy
 * sig: (Ljava/lang/Object;ILjava/lang/Object;II)V
 */
JNIEXPORT void JNICALL
Java_java_lang_System_arraycopy
    (JNIEnv *env, jclass klass, jobject, jint, jobject, jint, jint);

/**
 * class: java/lang/System
 * method: currentTimeMillis
 * sig: ()J
 */
JNIEXPORT jlong JNICALL
Java_java_lang_System_currentTimeMillis
    (JNIEnv *env, jclass klass);

/**
 * class: java/lang/System
 * method: identityHashCode
 * sig: (Ljava/lang/Object;)I
 */
JNIEXPORT jint JNICALL
Java_java_lang_System_identityHashCode
    (JNIEnv *env, jclass klass, jobject);

/**
 * class: java/lang/System
 * method: initProperties
 * sig: (Ljava/util/Properties;)Ljava/util/Properties;
 */
JNIEXPORT jobject JNICALL
Java_java_lang_System_initProperties
    (JNIEnv *env, jclass klass, jobject);

/**
 * class: java/lang/System
 * method: mapLibraryName
 * sig: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL
Java_java_lang_System_mapLibraryName
    (JNIEnv *env, jclass klass, jstring);

/**
 * class: java/lang/System
 * method: nanoTime
 * sig: ()J
 */
JNIEXPORT jlong JNICALL
Java_java_lang_System_nanoTime
    (JNIEnv *env, jclass klass);

/**
 * class: java/lang/System
 * method: setErr0
 * sig: (Ljava/io/PrintStream;)V
 */
JNIEXPORT void JNICALL
Java_java_lang_System_setErr0
    (JNIEnv *env, jclass klass, jobject);

/**
 * class: java/lang/System
 * method: setIn0
 * sig: (Ljava/io/InputStream;)V
 */
JNIEXPORT void JNICALL
Java_java_lang_System_setIn0
    (JNIEnv *env, jclass klass, jobject);

/**
 * class: java/lang/System
 * method: setOut0
 * sig: (Ljava/io/PrintStream;)V
 */
JNIEXPORT void JNICALL
Java_java_lang_System_setOut0
    (JNIEnv *env, jclass klass, jobject);



#ifdef __cplusplus
}
#endif
#endif // JAVA_JAVA_LANG_SYSTEM_H
