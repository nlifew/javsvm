


static jstring (JNICALL NewString)
        (JNIEnv *, const jchar *unicode, jsize len)
{
    auto &string = javsvm::jvm::get().string;

    if (unicode == nullptr || len <= 0) {
        return (jstring) to_object(string.new_string(""));
    }
    auto actually_len = wcslen((wchar_t *) unicode);
    if (actually_len <= len) {
        return (jstring) to_object(string.new_string((wchar_t *) unicode));
    }
    auto buff = new wchar_t[len + 1];
    std::unique_ptr<wchar_t, void(*)(const wchar_t *)> buff_guard(
            buff, [](const wchar_t *p) { delete[] p; });

    memcpy(buff, unicode, sizeof(wchar_t) * len);
    buff[len] = 0;

    return (jstring) to_object(string.new_string((wchar_t *) buff));
}


static jsize (JNICALL GetStringLength)
        (JNIEnv *, jstring str)
{
    return javsvm::jvm::get().string.length(to_object(str));
}


static const jchar * (JNICALL GetStringCritical)
        (JNIEnv *env, jstring string, jboolean *isCopy)
{
    auto value = javsvm::jvm::get().string.value_of(to_object(string));
    return (jchar *) GetPrimitiveArrayCritical(env, (jarray) to_object(value), isCopy);
}

static void (JNICALL ReleaseStringCritical)
        (JNIEnv *env, jstring string, const jchar *cstring)
{
    auto value = javsvm::jvm::get().string.value_of(to_object(string));
    ReleasePrimitiveArrayCritical(env, (jarray) to_object(value), (void *) cstring, 0);
}


static const jchar *(JNICALL GetStringChars)
        (JNIEnv *env, jstring str, jboolean *isCopy)
{
    auto value = javsvm::jvm::get().string.value_of(to_object(str));
    return (jchar*) GetCharArrayElements(env, (jcharArray) to_object(value), isCopy);
}

static void (JNICALL ReleaseStringChars)
        (JNIEnv *env, jstring str, const jchar *chars)
{
    auto value = javsvm::jvm::get().string.value_of(to_object(str));
    ReleaseCharArrayElements(env, (jcharArray) to_object(value), (jchar*) chars, JNI_ABORT);
}


static jstring (JNICALL NewStringUTF)
        (JNIEnv *, const char *utf)
{
    auto _ret = javsvm::jvm::get().string.new_string(utf);
    return (jstring) to_object(_ret);
}

static jsize (JNICALL GetStringUTFLength)
        (JNIEnv *env, jstring str)
{
    auto buff = GetStringCritical(env, str, nullptr);
    if (buff == nullptr) {
        return 0;
    }
    char *utf = nullptr;
    auto len = javsvm::strings::tostring((wchar_t *) buff, &utf);
    ReleaseStringCritical(env, str, buff);
    delete[] utf;
    return (jsize) len;
}


static const char* (JNICALL GetStringUTFChars)
        (JNIEnv *env, jstring str, jboolean *isCopy)
{
    if (*isCopy) *isCopy = true;

    auto buff = GetStringCritical(env, str, nullptr);
    if (buff == nullptr) {
        return nullptr;
    }
    char *utf = nullptr;
    javsvm::strings::tostring((wchar_t *) buff, &utf);
    ReleaseStringCritical(env, str, buff);

    return utf;
}

static void (JNICALL ReleaseStringUTFChars)
        (JNIEnv *, jstring, const char* chars)
{
    delete[] chars;
}

static void (JNICALL GetStringRegion)
        (JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf)
{
    auto value = javsvm::jvm::get().string.value_of(to_object(str));
    GetCharArrayRegion(env, (jcharArray) to_object(value), start, len, buf);
}

static void (JNICALL GetStringUTFRegion)
        (JNIEnv *env, jstring str, jsize start, jsize len, char *buf)
{
    buf[0] = '\0';

    if (len <= 0) {
        return;
    }

    auto buff = GetStringCritical(env, str, nullptr);
    if (buff != nullptr) {
        // 认为传进来的参数已经包含了足够大的缓冲区，因此只需要写进去就可以
        mbstowcs((wchar_t *) buff, buf, len * 4);
        ReleaseStringCritical(env, str, buff);
    }
}

