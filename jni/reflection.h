
static jmethodID (JNICALL FromReflectedMethod)
        (JNIEnv *env, jobject method)
{
    // TODO: 暂时不支持反射
    return nullptr;
}

static jfieldID (JNICALL FromReflectedField)
        (JNIEnv *env, jobject field)
{
    // TODO: 暂时不支持反射
    return nullptr;
}


static jobject (JNICALL ToReflectedMethod)
        (JNIEnv *env, jclass cls, jmethodID methodID, jboolean isStatic)
{
    // TODO: 暂时不支持反射
    return nullptr;
}



static jobject (JNICALL ToReflectedField)
        (JNIEnv *env, jclass cls, jfieldID fieldID, jboolean isStatic)
{
    // TODO: 暂时不支持反射
    return nullptr;
}