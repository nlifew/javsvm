
#include "jni_utils.h"
#include "jni_env.h"
#include "../utils/strings.h"

namespace jni
{

jstring (JNICALL NewString)
        (JNIEnv *, const jchar *unicode, jsize len)
{
    safety_area_guard guard;

    auto &string = javsvm::jvm::get().string;
    return to_object<jstring>(string.new_string(unicode, len));
}


jsize (JNICALL GetStringLength)
        (JNIEnv *, jstring str)
{
    safety_area_guard guard;
    return javsvm::jvm::get().string.length(to_object(str));
}

static inline jcharArray value_of(jstring str) noexcept
{
    safety_area_guard guard;
    return to_object<jcharArray>(javsvm::jstring::value_of(to_object(str)));
}

const jchar *(JNICALL GetStringCritical)
        (JNIEnv *env, jstring str, jboolean *isCopy)
{
    return (jchar *) GetPrimitiveArrayCritical(env, value_of(str), isCopy);
}

void (JNICALL ReleaseStringCritical)
        (JNIEnv *env, jstring str, const jchar *cstr) {
    ReleasePrimitiveArrayCritical(env, value_of(str), (void *) cstr, 0);
}


const jchar *(JNICALL GetStringChars)
        (JNIEnv *env, jstring str, jboolean *isCopy)
{
    return (jchar *) GetCharArrayElements(env, value_of(str), isCopy);
}

void (JNICALL ReleaseStringChars)
        (JNIEnv *env, jstring str, const jchar *chars)
{
    ReleaseCharArrayElements(env, value_of(str), (jchar *) chars, JNI_ABORT);
}


jstring (JNICALL NewStringUTF)
        (JNIEnv *, const char *utf)
{
    safety_area_guard guard;

    auto _ret = javsvm::jvm::get().string.new_string(utf);
    return (jstring) to_object(_ret);
}

jsize (JNICALL GetStringUTFLength)
        (JNIEnv *env, jstring str)
{
    auto utf8 = GetStringUTFChars(env, str, nullptr);
    size_t size = strlen(utf8);
    ReleaseStringUTFChars(env, str, utf8);
    return (jsize) size;
}


const char *(JNICALL GetStringUTFChars)
        (JNIEnv *, jstring str, jboolean *isCopy)
{
    safety_area_guard guard;

    if (*isCopy) *isCopy = true;

    return javsvm::jstring::utf8(to_object(str));
}

void (JNICALL ReleaseStringUTFChars)
        (JNIEnv *, jstring, const char *chars) {
    delete[] chars;
}

void (JNICALL GetStringRegion)
        (JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf) {
    GetCharArrayRegion(env, value_of(str), start, len, buf);
}

void (JNICALL GetStringUTFRegion)
        (JNIEnv *, jstring str, jsize start, jsize len, char *buf) {
    buf[0] = '\0';

    if (len <= 0) {
        return;
    }

    safety_area_guard guard;

    auto value = javsvm::jstring::value_of(to_object(str));

    int array_len, ele_size;
    auto mem = (jchar *) javsvm::jarray::storage_of(javsvm::jheap::cast(value), &array_len, &ele_size);

    // 判断数组是否越界
    if (start < 0 || start + len > array_len) {
        javsvm::throw_exp("java/lang/IndexOutOfBoundsException", "");
        return;
    }

    javsvm::strings::to_string(buf, mem + start, len);
}

}