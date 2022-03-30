
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
    return to_object<jstring>(string.new_string((wchar_t *) unicode, len));
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
    auto buff = GetStringCritical(env, str, nullptr);
    if (buff == nullptr) {
        return 0;
    }

    size_t len;
    {
        safety_area_guard guard;
        char *utf = nullptr;
        len = javsvm::strings::tostring((wchar_t *) buff, &utf);
        delete[] utf;
    }

    ReleaseStringCritical(env, str, buff);
    return (jsize) len;
}


const char *(JNICALL GetStringUTFChars)
        (JNIEnv *env, jstring str, jboolean *isCopy) {
    if (*isCopy) *isCopy = true;

    auto buff = GetStringCritical(env, str, nullptr);
    if (buff == nullptr) {
        return nullptr;
    }

    char *utf = nullptr;
    {
        safety_area_guard guard;
        javsvm::strings::tostring((wchar_t *) buff, &utf);
    }

    ReleaseStringCritical(env, str, buff);
    return utf;
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
        (JNIEnv *env, jstring str, jsize start, jsize len, char *buf) {
    buf[0] = '\0';

    if (len <= 0) {
        return;
    }

    auto buff = GetStringCritical(env, str, nullptr);
    if (buff != nullptr) {
        // 认为传进来的参数已经包含了足够大的缓冲区，因此只需要写进去就可以
        mbstowcs((wchar_t *) buff + start, buf, len * 4);
        ReleaseStringCritical(env, str, buff);
    }
}

}