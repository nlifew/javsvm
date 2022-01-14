
static jobject (JNICALL NewGlobalRef)
        (JNIEnv *env, jobject lobj)
{
    // todo
    return lobj;
}

static void (DeleteGlobalRef)
        (JNIEnv *env, jobject gref)
{
    // todo
}

static void (JNICALL DeleteLocalRef)
        (JNIEnv *env, jobject obj)
{
    // todo
}


static jobject (JNICALL NewLocalRef)
        (JNIEnv *env, jobject ref)
{
    // todo
    return ref;
}

static jint (JNICALL EnsureLocalCapacity)
        (JNIEnv *env, jint capacity)
{
    // todo
    return 0;
}


static jweak (JNICALL NewWeakGlobalRef)
        (JNIEnv *env, jobject obj)
{
    // todo
    return obj;
}

static void (JNICALL DeleteWeakGlobalRef)
        (JNIEnv *env, jweak ref)
{
    // todo
}

/* New JNI 1.6 Features */

static jobjectRefType (JNICALL GetObjectRefType)
        (JNIEnv* env, jobject obj)
{
    // todo
    return JNIGlobalRefType;
}

