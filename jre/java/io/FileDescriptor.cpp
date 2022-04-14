


#include "jni/jni_env.h"
#include "java/io/FileDescriptor.h"

#include <unistd.h>
#include <cerrno>
#include <cstring>

/**
 * class: java/io/FileDescriptor
 * method: sync
 * sig: ()V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_io_FileDescriptor_sync
    (JNIEnv *env, jobject self)
{
    jclass klass = jni::GetObjectClass(env, self);
    jfieldID _fd = jni::GetFieldID(env, klass, "fd", "I");
    jint fd = jni::GetIntField(env, self, _fd);

    int result = fsync(fd);

    if (result < 0) {
        jclass SyncFailedException = jni::FindClass(env, "java/io/SyncFailedException");
        char msg[256];
        snprintf(msg, sizeof(msg), "failed to sync %d cause %s\n",
                 fd, strerror(errno));
        jni::ThrowNew(env, SyncFailedException, msg);
    }
}

