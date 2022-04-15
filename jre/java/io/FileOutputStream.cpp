
#include "jni/jni_env.h"
#include "java/io/FileOutputStream.h"

#include <unistd.h>
#include <fcntl.h>
#include <cstdarg>

struct FileDescriptor_t
{
    jfieldID fd = nullptr;
};

struct FileOutputStream_t
{
    jfieldID fd = nullptr;
};

static FileDescriptor_t FileDescriptor;
static FileOutputStream_t FileOutputStream;


static int get_fd(JNIEnv *env, jobject self) noexcept
{
    jobject fd = jni::GetObjectField(env, self, FileOutputStream.fd);
    return jni::GetIntField(env, fd, FileDescriptor.fd);
}

static void set_fd(JNIEnv *env, jobject self, int val) noexcept
{
    jobject fd = jni::GetObjectField(env, self, FileOutputStream.fd);
    jni::SetIntField(env, fd, FileDescriptor.fd, val);
}

#define IOException "java/io/IOException"

static void throw_ioe(const char *ioe, JNIEnv *env, const char *fmt, ...) noexcept
{
    char buff[1024];

    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buff, sizeof(buff), fmt, ap);
    va_end(ap);

    jclass _ioe = jni::FindClass(env, ioe);
    jni::ThrowNew(env, _ioe, buff);
}


/**
 * class: java/io/FileOutputStream
 * method: close0
 * sig: ()V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_close0
    (JNIEnv *env, jobject self)
{
    int fd = get_fd(env, self);
    if (fd < 0) {
        return;
    }
    int ret = close(fd);
    if (ret < 0) {
        throw_ioe(IOException, env, "failed to close fd %d", fd);
        return;
    }
    set_fd(env, self, -1);
}

/**
 * class: java/io/FileOutputStream
 * method: initIDs
 * sig: ()V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_initIDs
    (JNIEnv *env, jclass klass)
{
    jclass java_io_FileDescriptor = jni::FindClass(env, "java/io/FileDescriptor");
    jclass java_io_FileOutputStream = klass; // jni::FindClass(env, "java/io/FileInputStream");

    FileDescriptor.fd = jni::GetFieldID(env, java_io_FileDescriptor, "fd", "I");
    FileOutputStream.fd = jni::GetFieldID(env, java_io_FileOutputStream, "fd", "Ljava/io/FileDescriptor;");
}

/**
 * class: java/io/FileOutputStream
 * method: open0
 * sig: (Ljava/lang/String;Z)V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_open0
    (JNIEnv *env, jobject self, jstring name, jboolean append)
{
    const char *name_s = jni::GetStringUTFChars(env, name, nullptr);
    if (name_s == nullptr) {
        return;
    }

    int fd = open(name_s, O_WRONLY | O_CREAT | (append ? O_APPEND : O_TRUNC));
    if (fd < 0) {
        throw_ioe(IOException, env, "failed to open file '%s'\n", name_s);
    }
    set_fd(env, self, fd);
    jni::ReleaseStringUTFChars(env, name, name_s);
}

/**
 * class: java/io/FileOutputStream
 * method: writeBytes
 * sig: ([BIIZ)V
 */
JNIEXPORT void JNICALL
Java_java_io_FileOutputStream_writeBytes
    (JNIEnv *env, jobject self, jbyteArray out, jint off, jint len, jboolean)
{
    int fd = get_fd(env, self);
    if (fd < 0) {
        throw_ioe(IOException, env, "invalid fd %d\n", fd);
        return;
    }

    auto buff = (jbyte *) jni::GetPrimitiveArrayCritical(env, out, nullptr);
    auto ret = write(fd, buff + off, len);
    jni::ReleasePrimitiveArrayCritical(env, out, buff, JNI_COMMIT);

    if (ret < 0) {
        // 出错
        throw_ioe(IOException, env, "failed to read %d bytes from fd %d", len, fd);
    }
}
