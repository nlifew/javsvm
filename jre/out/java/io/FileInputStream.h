/**
 * auto generated by javsvm-javah, don't modify it.
 */

#ifndef JAVA_JAVA_IO_FILEINPUTSTREAM_H
#define JAVA_JAVA_IO_FILEINPUTSTREAM_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * class: java/io/FileInputStream
 * method: available0
 * sig: ()I
 */
JNIEXPORT jint JNICALL
Java_java_io_FileInputStream_available0
    (JNIEnv *env, jobject self);

/**
 * class: java/io/FileInputStream
 * method: close0
 * sig: ()V
 */
JNIEXPORT void JNICALL
Java_java_io_FileInputStream_close0
    (JNIEnv *env, jobject self);

/**
 * class: java/io/FileInputStream
 * method: initIDs
 * sig: ()V
 */
JNIEXPORT void JNICALL
Java_java_io_FileInputStream_initIDs
    (JNIEnv *env, jclass klass);

/**
 * class: java/io/FileInputStream
 * method: open0
 * sig: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL
Java_java_io_FileInputStream_open0
    (JNIEnv *env, jobject self, jstring);

/**
 * class: java/io/FileInputStream
 * method: readBytes
 * sig: ([BII)I
 */
JNIEXPORT jint JNICALL
Java_java_io_FileInputStream_readBytes
    (JNIEnv *env, jobject self, jbyteArray, jint, jint);

/**
 * class: java/io/FileInputStream
 * method: skip0
 * sig: (J)J
 */
JNIEXPORT jlong JNICALL
Java_java_io_FileInputStream_skip0
    (JNIEnv *env, jobject self, jlong);



#ifdef __cplusplus
}
#endif
#endif // JAVA_JAVA_IO_FILEINPUTSTREAM_H

