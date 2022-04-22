
#include "jni/jni_utils.h"
#include "jni/jni_env.h"
#include "java/lang/System.h"
#include "object/jfield.h"

#include <chrono>

/**
 * class: java/lang/System
 * method: arraycopy
 * sig: (Ljava/lang/Object;ILjava/lang/Object;II)V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_System_arraycopy
    (JNIEnv *, jclass, jobject src, jint src_off, jobject dst, jint dst_off, jint len)
{
    safety_area_guard guard;

    // 判空
    auto src_obj = javsvm::jheap::cast(to_object(src));
    auto dst_obj = javsvm::jheap::cast(to_object(dst));

    if (src_obj == nullptr || dst_obj == nullptr) {
        javsvm::throw_exp("java/lang/NullPointerException", "");
        return;
    }

    // 判断是否是数组类型
    auto src_klass = src_obj->klass;
    auto dst_klass = dst_obj->klass;

    if (src_klass->component_type == nullptr || dst_klass->component_type == nullptr) {
        javsvm::throw_exp("java/lang/ArrayStoreException", "");
        return;
    }
    // 如果是不兼容的数组，抛出异常
    if (! dst_klass->is_assign_from(src_klass)) {
        javsvm::throw_exp("java/lang/ArrayStoreException", "");
        return;
    }

    jint src_len, src_ele_size;
    char *src_mem = (char*) javsvm::jarray::storage_of(src_obj, &src_len, &src_ele_size);

    jint dst_len, dst_ele_size;
    char *dst_mem = (char*) javsvm::jarray::storage_of(dst_obj, &dst_len, &dst_ele_size);

    // 判断数组越界异常
    if (len < 0 || src_off < 0 || (src_off + len) > src_len || dst_off < 0 || (dst_off + len) > dst_len) {
        char msg[512];
        snprintf(msg, sizeof(msg), "srcPos: %d, dstPos: %d, len: %d, srcLen: %d, dstLen: %d\n",
                 src_off, dst_off, len, src_len, dst_len);
        javsvm::throw_exp("java/lang/IndexOutOfBoundsException", msg);
        return;
    }

    assert(src_ele_size == dst_ele_size);

    // 如果 src 和 dst 同是一个对象，用 memmove()
    if (src_obj == dst_obj) {
        memmove(dst_mem + dst_ele_size * dst_off, src_mem + src_ele_size * src_off, len * src_ele_size);
    }
    else {
        memcpy(dst_mem + dst_ele_size * dst_off, src_mem + src_ele_size * src_off, len * src_ele_size);
    }
}

/**
 * class: java/lang/System
 * method: currentTimeMillis
 * sig: ()J
 */
extern "C" JNIEXPORT jlong JNICALL
Java_java_lang_System_currentTimeMillis
    (JNIEnv *, jclass)
{
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
    return t.time_since_epoch().count();
}


/**
 * class: java/lang/System
 * method: nanoTime
 * sig: ()J
 */
extern "C" JNIEXPORT jlong JNICALL
        Java_java_lang_System_nanoTime
        (JNIEnv *, jclass)
{
    auto now = std::chrono::system_clock::now();
    auto t = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    return t.time_since_epoch().count();
}

/**
 * class: java/lang/System
 * method: identityHashCode
 * sig: (Ljava/lang/Object;)I
 */
extern "C" JNIEXPORT jint JNICALL
Java_java_lang_System_identityHashCode
    (JNIEnv *, jclass, jobject obj)
{
    safety_area_guard guard;
    auto ptr = javsvm::jheap::cast(to_object(obj));
    return ptr == nullptr ? 0 : ptr->hash_code();
}

/**
 * class: java/lang/System
 * method: mapLibraryName
 * sig: (Ljava/lang/String;)Ljava/lang/String;
 */
extern "C" JNIEXPORT jstring JNICALL
Java_java_lang_System_mapLibraryName
    (JNIEnv *env, jclass, jstring name)
{
    scoped_string name_s(env, name);

    std::string s;
#if platform_os_arch == platform_windows_x64
    s += name_s.get();
    s += ".dll";
#elif platform_os_arch == platform_macos_x64 || platform_os_arch == platform_macos_arm64
    s += "lib";
    s += name_s.get();
    s += ".dylib";
#elif platform_os_arch == platform_linux_x64
    s += "lib";
    s += name_s.get();
    s += ".so";
#endif
    return jni::NewStringUTF(env, s.c_str());
}


/**
 * class: java/lang/System
 * method: setIn0
 * sig: (Ljava/io/InputStream;)V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_System_setIn0
        (JNIEnv *, jclass klass, jobject is)
{
    safety_area_guard guard;
    auto java_lang_System = to_class(klass);
    auto in = java_lang_System->get_static_field("in", "Ljava/io/InputStream;");

    javsvm::jvalue val { .l = to_object(is), };
    in->set_static(val);

    // todo: 我们要同步到 stdin 吗 ?
}


/**
 * class: java/lang/System
 * method: setOut0
 * sig: (Ljava/io/PrintStream;)V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_System_setOut0
    (JNIEnv *, jclass klass, jobject ps)
{
    safety_area_guard guard;
    auto java_lang_System = to_class(klass);
    auto out = java_lang_System->get_static_field("out", "Ljava/io/PrintStream;");

    javsvm::jvalue val { .l = to_object(ps), };
    out->set_static(val);

    // todo: 我们要同步到 stdout 吗 ?
}


/**
 * class: java/lang/System
 * method: setErr0
 * sig: (Ljava/io/PrintStream;)V
 */
extern "C" JNIEXPORT void JNICALL
Java_java_lang_System_setErr0
        (JNIEnv *, jclass klass, jobject ps)
{
    safety_area_guard guard;
    auto java_lang_System = to_class(klass);
    auto err = java_lang_System->get_static_field("err", "Ljava/io/PrintStream;");

    javsvm::jvalue val { .l = to_object(ps), };
    err->set_static(val);

    // todo: 我们要同步到 stderr 吗 ?
}



struct java_util_Properties
{
private:
    JNIEnv *m_env;
    jobject m_this;
    jmethodID _setProperty;
public:
    java_util_Properties(JNIEnv *env, jobject self) noexcept:
        m_env(env),
        m_this(self)
    {
        auto klass = jni::GetObjectClass(env, self);

        _setProperty = jni::GetMethodID(env, klass,"setProperty",
                                        "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/Object;");
        assert(_setProperty != nullptr);
    }

    jobject setProperty(const char *key, const char *value) noexcept
    {
        auto _key = jni::NewStringUTF(m_env, key);
        auto _value = jni::NewStringUTF(m_env, value);
        return jni::CallObjectMethod(m_env, m_this, _setProperty, _key, _value);
    }
};


/**
 * class: java/lang/System
 * method: initProperties
 * sig: (Ljava/util/Properties;)Ljava/util/Properties;
 */
extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_System_initProperties
        (JNIEnv *env, jclass, jobject prop)
{
#if platform_os_arch == platform_windows_x64
    const char *path_split = ";";
    const char *file_split = "\\";
    const char *line_split = "\r\n";
#else
    const char *path_split = ":";
    const char *file_split = "/";
    const char *line_split = "\n";
#endif
    // todo: usr library, boot library, ...

    java_util_Properties props(env, prop);
    props.setProperty("path.separator", path_split);
    props.setProperty("file.separator", file_split);
    props.setProperty("line.separator", line_split);

    return prop;
}
