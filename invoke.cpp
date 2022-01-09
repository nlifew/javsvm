# 1 "jni/invoke.h"
# 1 "<built-in>" 1
# 1 "<built-in>" 3
# 397 "<built-in>" 3
# 1 "<command line>" 1
# 1 "<built-in>" 2
# 1 "jni/invoke.h" 2




static jmethodID (JNICALL GetMethodID)
        (JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
}

static jvalue invoke_method(JNIEnv *env, jobject obj, jmethodID methodId, const jvalue *value)
{

}

static jvalue *varg2list(jmethodID method, va_list ap)
{

}






# 1 "jni/invoke_gen.h" 1
# 14 "jni/invoke_gen.h"
static jobject (JNICALL CallObjectMethodA)
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    jvalue value = invoke_method(env, obj, methodID, args);
    return take_from<jobject>(value);
}


static jobject (JNICALL CallObjectMethodV)
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto args_list = varg2list(methodID, args);
    auto ret = CallObjectMethodA(env, obj, methodID, args_list);
    delete[] args_list;
    return ret;
}

static jobject (JNICALL CallObjectMethod)
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = CallObjectMethodV(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}
# 25 "jni/invoke.h" 2



# 1 "jni/invoke_gen.h" 1
# 14 "jni/invoke_gen.h"
static jboolean (JNICALL CallBooleanMethodA)
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    jvalue value = invoke_method(env, obj, methodID, args);
    return take_from<jboolean>(value);
}


static jboolean (JNICALL CallBooleanMethodV)
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto args_list = varg2list(methodID, args);
    auto ret = CallBooleanMethodA(env, obj, methodID, args_list);
    delete[] args_list;
    return ret;
}

static jboolean (JNICALL CallBooleanMethod)
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = CallBooleanMethodV(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}
# 29 "jni/invoke.h" 2




# 1 "jni/invoke_gen.h" 1
# 14 "jni/invoke_gen.h"
static jbyte (JNICALL CallByteMethodA)
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    jvalue value = invoke_method(env, obj, methodID, args);
    return take_from<jbyte>(value);
}


static jbyte (JNICALL CallByteMethodV)
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto args_list = varg2list(methodID, args);
    auto ret = CallByteMethodA(env, obj, methodID, args_list);
    delete[] args_list;
    return ret;
}

static jbyte (JNICALL CallByteMethod)
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = CallByteMethodV(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}
# 34 "jni/invoke.h" 2




# 1 "jni/invoke_gen.h" 1
# 14 "jni/invoke_gen.h"
static jchar (JNICALL CallCharMethodA)
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    jvalue value = invoke_method(env, obj, methodID, args);
    return take_from<jchar>(value);
}


static jchar (JNICALL CallCharMethodV)
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto args_list = varg2list(methodID, args);
    auto ret = CallCharMethodA(env, obj, methodID, args_list);
    delete[] args_list;
    return ret;
}

static jchar (JNICALL CallCharMethod)
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = CallCharMethodV(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}
# 39 "jni/invoke.h" 2




# 1 "jni/invoke_gen.h" 1
# 14 "jni/invoke_gen.h"
static jshort (JNICALL CallShortMethodA)
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    jvalue value = invoke_method(env, obj, methodID, args);
    return take_from<jshort>(value);
}


static jshort (JNICALL CallShortMethodV)
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto args_list = varg2list(methodID, args);
    auto ret = CallShortMethodA(env, obj, methodID, args_list);
    delete[] args_list;
    return ret;
}

static jshort (JNICALL CallShortMethod)
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = CallShortMethodV(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}
# 44 "jni/invoke.h" 2




# 1 "jni/invoke_gen.h" 1
# 14 "jni/invoke_gen.h"
static jint (JNICALL CallIntMethodA)
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    jvalue value = invoke_method(env, obj, methodID, args);
    return take_from<jint>(value);
}


static jint (JNICALL CallIntMethodV)
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto args_list = varg2list(methodID, args);
    auto ret = CallIntMethodA(env, obj, methodID, args_list);
    delete[] args_list;
    return ret;
}

static jint (JNICALL CallIntMethod)
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = CallIntMethodV(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}
# 49 "jni/invoke.h" 2




# 1 "jni/invoke_gen.h" 1
# 14 "jni/invoke_gen.h"
static jlong (JNICALL CallLongMethodA)
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    jvalue value = invoke_method(env, obj, methodID, args);
    return take_from<jlong>(value);
}


static jlong (JNICALL CallLongMethodV)
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto args_list = varg2list(methodID, args);
    auto ret = CallLongMethodA(env, obj, methodID, args_list);
    delete[] args_list;
    return ret;
}

static jlong (JNICALL CallLongMethod)
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = CallLongMethodV(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}
# 54 "jni/invoke.h" 2




# 1 "jni/invoke_gen.h" 1
# 14 "jni/invoke_gen.h"
static jfloat (JNICALL CallFloatMethodA)
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    jvalue value = invoke_method(env, obj, methodID, args);
    return take_from<jfloat>(value);
}


static jfloat (JNICALL CallFloatMethodV)
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto args_list = varg2list(methodID, args);
    auto ret = CallFloatMethodA(env, obj, methodID, args_list);
    delete[] args_list;
    return ret;
}

static jfloat (JNICALL CallFloatMethod)
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = CallFloatMethodV(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}
# 59 "jni/invoke.h" 2




# 1 "jni/invoke_gen.h" 1
# 14 "jni/invoke_gen.h"
static jdouble (JNICALL CallDoubleMethodA)
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    jvalue value = invoke_method(env, obj, methodID, args);
    return take_from<jdouble>(value);
}


static jdouble (JNICALL CallDoubleMethodV)
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto args_list = varg2list(methodID, args);
    auto ret = CallDoubleMethodA(env, obj, methodID, args_list);
    delete[] args_list;
    return ret;
}

static jdouble (JNICALL CallDoubleMethod)
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = CallDoubleMethodV(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}
# 64 "jni/invoke.h" 2




# 1 "jni/invoke_gen.h" 1
# 14 "jni/invoke_gen.h"
static void (JNICALL CallVoidMethodA)
        (JNIEnv *env, jobject obj, jmethodID methodID, const jvalue * args)
{
    jvalue value = invoke_method(env, obj, methodID, args);
    return take_from<void>(value);
}


static void (JNICALL CallVoidMethodV)
        (JNIEnv *env, jobject obj, jmethodID methodID, va_list args)
{
    auto args_list = varg2list(methodID, args);
    auto ret = CallVoidMethodA(env, obj, methodID, args_list);
    delete[] args_list;
    return ret;
}

static void (JNICALL CallVoidMethod)
        (JNIEnv *env, jobject obj, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = CallVoidMethodV(env, obj, methodID, ap);
    va_end(ap);
    return ret;
}
# 69 "jni/invoke.h" 2

