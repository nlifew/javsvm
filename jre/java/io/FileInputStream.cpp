
#include "jni/jni_utils.h"
#include "jni/jni_env.h"
#include "java/io/FileInputStream.h"

#include <unistd.h>
#include <fcntl.h>
#include <cstdarg>

struct FileDescriptor_t
{
    jfieldID fd = nullptr;
};

struct InputStreamReader_t
{
    jfieldID fd = nullptr;
};

static FileDescriptor_t FileDescriptor;
static InputStreamReader_t InputStreamReader;


static int get_fd(JNIEnv *env, jobject self) noexcept
{
    jobject fd = jni::GetObjectField(env, self, InputStreamReader.fd);
    return jni::GetIntField(env, fd, FileDescriptor.fd);
}

static void set_fd(JNIEnv *env, jobject self, int val) noexcept
{
    jobject fd = jni::GetObjectField(env, self, InputStreamReader.fd);
    jni::SetIntField(env, fd, FileDescriptor.fd, val);
}

#define IOException "java/io/IOException"
#define EOFException "java/io/EOFException"
#define FileNotFoundException "java/io/FileNotFoundException"

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
 * class: java/io/FileInputStream
 * method: available0
 * sig: ()I
 */
extern "C" JNIEXPORT jint JNICALL
Java_java_io_FileInputStream_available0
    (JNIEnv *env, jobject self)
{
    int fd = get_fd(env, self);
    if (fd < 0) {
        throw_ioe(IOException, env, "invalid fd %d", fd);
        return -1;
    }

    off_t cur = lseek(fd, 0, SEEK_CUR);
    off_t end = lseek(fd, 0, SEEK_END);

    if (cur < 0 || end < 0) {
        throw_ioe(IOException, env, "failed to access file offset of %d\n", fd);
        return -1;
    }
    lseek(fd, cur, SEEK_SET);
    return (jint) (end - cur);
}

/**
 * class: java/io/FileInputStream
 * method: close0
 * sig: ()V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_io_FileInputStream_close0
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
 * class: java/io/FileInputStream
 * method: initIDs
 * sig: ()V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_io_FileInputStream_initIDs
    (JNIEnv *env, jclass klass)
{
    jclass java_io_FileDescriptor = jni::FindClass(env, "java/io/FileDescriptor");
    jclass java_io_FileInputStream = klass; // jni::FindClass(env, "java/io/FileInputStream");

    FileDescriptor.fd = jni::GetFieldID(env, java_io_FileDescriptor, "fd", "I");
    InputStreamReader.fd = jni::GetFieldID(env, java_io_FileInputStream, "fd", "Ljava/io/FileDescriptor;");
}

/**
 * class: java/io/FileInputStream
 * method: open0
 * sig: (Ljava/lang/String;)V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_io_FileInputStream_open0
    (JNIEnv *env, jobject self, jstring name)
{
    const char *name_s = jni::GetStringUTFChars(env, name, nullptr);
    if (jni::ExceptionCheck(env)) {
        return;
    }

    int fd = open(name_s, O_RDONLY);
    if (fd < 0) {
        throw_ioe(FileNotFoundException, env, "failed to open file '%s'\n", name_s);
    }
    set_fd(env, self, fd);
    jni::ReleaseStringUTFChars(env, name, name_s);
}

/**
 * class: java/io/FileInputStream
 * method: readBytes
 * sig: ([BII)I
 */
extern "C" JNIEXPORT jint JNICALL
Java_java_io_FileInputStream_readBytes
    (JNIEnv *env, jobject self, jbyteArray out, jint off, jint len)
{
    int fd = get_fd(env, self);
    if (fd < 0) {
        throw_ioe(IOException, env, "invalid fd %d\n", fd);
        return -1;
    }
    auto buff = (jbyte *) jni::GetPrimitiveArrayCritical(env, out, nullptr);
    auto ret = read(fd, buff + off, len);
    jni::ReleasePrimitiveArrayCritical(env, out, buff, JNI_COMMIT);

    if (ret < 0) {
        // 出错
        throw_ioe(IOException, env, "failed to read %d bytes from fd %d", len, fd);
        return -1;
    }
    if (ret == 0) {
        // 表示已经到达文件末尾
        return -1;
    }
    return (jint) ret;
}

/**
 * class: java/io/FileInputStream
 * method: skip0
 * sig: (J)J
 */
extern "C" JNIEXPORT jlong JNICALL
Java_java_io_FileInputStream_skip0
    (JNIEnv *env, jobject self, jlong skip)
{
    int fd = get_fd(env, self);
    if (fd < 0) {
        throw_ioe(IOException, env, "invalid fd %d\n", fd);
        return -1;
    }
    auto cur = lseek(fd, 0, SEEK_CUR);
    auto ret = lseek(fd, skip, SEEK_CUR);
    if (ret < 0) {
        throw_ioe(IOException, env, "failed to seek %d bytes from fd %d\n", skip, fd);
        return -1;
    }
    return ret - cur;
}

