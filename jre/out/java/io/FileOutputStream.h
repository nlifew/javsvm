/**
 * auto generated by javsvm-javah, don't modify it.
 */

#ifndef JAVA_JAVA_IO_FILEOUTPUTSTREAM_H
#define JAVA_JAVA_IO_FILEOUTPUTSTREAM_H

#include <jni.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * class: java/io/FileOutputStream
 * method: close0
 * sig: ()V
 */
JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_close0
    (JNIEnv *env, jobject self);

/**
 * class: java/io/FileOutputStream
 * method: initIDs
 * sig: ()V
 */
JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_initIDs
    (JNIEnv *env, jclass klass);

/**
 * class: java/io/FileOutputStream
 * method: open0
 * sig: (Ljava/lang/String;Z)V
 */
JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_open0
    (JNIEnv *env, jobject self, jstring, jboolean);

/**
 * class: java/io/FileOutputStream
 * method: writeBytes
 * sig: ([BIIZ)V
 */
JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_writeBytes
    (JNIEnv *env, jobject self, jbyteArray, jint, jint, jboolean);



#ifdef __cplusplus
}
#endif
#endif // JAVA_JAVA_IO_FILEOUTPUTSTREAM_H
