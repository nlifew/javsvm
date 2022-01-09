

#include <cstdarg>

#include "jni.h"
#include "jni_utils.h"



#include "call_method.h"

struct _jfieldID
{
};

struct _jmethodID
{
};



static jint JNICALL
GetVersion(JNIEnv *env)
{
    // 只支持常量，jni 1.8 接口
    return JNI_VERSION_1_8;
}


static jclass (JNICALL DefineClass)
        (JNIEnv *env, const char *name, jobject loader, const jbyte *buf,
         jsize len)
{
    return nullptr;
}

static jclass (JNICALL FindClass)
        (JNIEnv *env, const char *name)
{
    // todo:
}


static jclass (JNICALL GetSuperclass)
        (JNIEnv *env, jclass sub)
{

}

static jboolean (JNICALL IsAssignableFrom)
        (JNIEnv *env, jclass sub, jclass sup)
{
}

static jmethodID (JNICALL FromReflectedMethod)
        (JNIEnv *env, jobject method)
{
}

static jfieldID (JNICALL FromReflectedField)
        (JNIEnv *env, jobject field)
{
}


static jobject (JNICALL ToReflectedMethod)
        (JNIEnv *env, jclass cls, jmethodID methodID, jboolean isStatic)
{
}



static jobject (JNICALL ToReflectedField)
        (JNIEnv *env, jclass cls, jfieldID fieldID, jboolean isStatic)
{
}


static jint (JNICALL Throw)
        (JNIEnv *env, jthrowable obj)
{
}

static jint (JNICALL ThrowNew)
        (JNIEnv *env, jclass clazz, const char *msg)
{
}

static jthrowable (JNICALL ExceptionOccurred)
        (JNIEnv *env)
{
}

static void (JNICALL ExceptionDescribe)
        (JNIEnv *env)
{
}

static void (JNICALL ExceptionClear)
        (JNIEnv *env)
{
}

static void (JNICALL FatalError)
        (JNIEnv *env, const char *msg)
{
}


static jint (JNICALL PushLocalFrame)
        (JNIEnv *env, jint capacity)
{
}

static jobject (JNICALL PopLocalFrame)
        (JNIEnv *env, jobject result)
{
}


static jobject (JNICALL NewGlobalRef)
        (JNIEnv *env, jobject lobj)
{
}

static void (DeleteGlobalRef)
        (JNIEnv *env, jobject gref)
{
}

static void (JNICALL DeleteLocalRef)
        (JNIEnv *env, jobject obj)
{
}

static jboolean (JNICALL IsSameObject)
        (JNIEnv *env, jobject obj1, jobject obj2)
{
}

static jobject (JNICALL NewLocalRef)
        (JNIEnv *env, jobject ref)
{
}

static jint (JNICALL EnsureLocalCapacity)
        (JNIEnv *env, jint capacity)
{
}


static jobject (JNICALL AllocObject)
        (JNIEnv *env, jclass clazz)
{
}



static jobject (JNICALL NewObjectA)
        (JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args)
{

}


static jobject (JNICALL NewObjectV)
        (JNIEnv *env, jclass clazz, jmethodID methodID, va_list args)
{

}


static jobject (JNICALL NewObject)
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto r = NewObjectV(env, clazz, methodID, ap);
    va_end(ap);
    return r;
}


static jclass (JNICALL GetObjectClass)
        (JNIEnv *env, jobject obj)
{
}

static jboolean (JNICALL IsInstanceOf)
        (JNIEnv *env, jobject obj, jclass clazz)
{
}


static jfieldID (JNICALL GetFieldID)
        (JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
}


static jobject (JNICALL GetObjectField)
        (JNIEnv *env, jobject obj, jfieldID fieldID)
{
}

static jboolean (JNICALL GetBooleanField)
        (JNIEnv *env, jobject obj, jfieldID fieldID)
{
}

static jbyte (JNICALL GetByteField)
        (JNIEnv *env, jobject obj, jfieldID fieldID)
{
}

static jchar (JNICALL GetCharField)
        (JNIEnv *env, jobject obj, jfieldID fieldID)
{
}

static jshort (JNICALL GetShortField)
        (JNIEnv *env, jobject obj, jfieldID fieldID)
{
}

static jint (JNICALL GetIntField)
        (JNIEnv *env, jobject obj, jfieldID fieldID)
{
}

static jlong (JNICALL GetLongField)
        (JNIEnv *env, jobject obj, jfieldID fieldID)
{
}

static jfloat (JNICALL GetFloatField)
        (JNIEnv *env, jobject obj, jfieldID fieldID)
{
}

static jdouble (JNICALL GetDoubleField)
        (JNIEnv *env, jobject obj, jfieldID fieldID)
{
}


static void (JNICALL SetObjectField)
        (JNIEnv *env, jobject obj, jfieldID fieldID, jobject val)
{
}

static void (JNICALL SetBooleanField)
        (JNIEnv *env, jobject obj, jfieldID fieldID, jboolean val)
{
}

static void (JNICALL SetByteField)
        (JNIEnv *env, jobject obj, jfieldID fieldID, jbyte val)
{
}

static void (JNICALL SetCharField)
        (JNIEnv *env, jobject obj, jfieldID fieldID, jchar val)
{
}

static void (JNICALL SetShortField)
        (JNIEnv *env, jobject obj, jfieldID fieldID, jshort val)
{
}

static void (JNICALL SetIntField)
        (JNIEnv *env, jobject obj, jfieldID fieldID, jint val)
{
}

static void (JNICALL SetLongField)
        (JNIEnv *env, jobject obj, jfieldID fieldID, jlong val)
{
}

static void (JNICALL SetFloatField)
        (JNIEnv *env, jobject obj, jfieldID fieldID, jfloat val)
{
}

static void (JNICALL SetDoubleField)
        (JNIEnv *env, jobject obj, jfieldID fieldID, jdouble val)
{
}



static jfieldID (JNICALL GetStaticFieldID)
        (JNIEnv *env, jclass clazz, const char *name, const char *sig)
{
}

static jobject (JNICALL GetStaticObjectField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID)
{
}

static jboolean (JNICALL GetStaticBooleanField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID)
{
}

static jbyte (JNICALL GetStaticByteField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID)
{
}

static jchar (JNICALL GetStaticCharField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID)
{
}

static jshort (JNICALL GetStaticShortField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID)
{
}

static jint (JNICALL GetStaticIntField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID)
{
}

static jlong (JNICALL GetStaticLongField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID)
{
}

static jfloat (JNICALL GetStaticFloatField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID)
{
}

static jdouble (JNICALL GetStaticDoubleField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID)
{
}


static void (JNICALL SetStaticObjectField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jobject value)
{
}

static void (JNICALL SetStaticBooleanField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jboolean value)
{
}

static void (JNICALL SetStaticByteField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jbyte value)
{
}

static void (JNICALL SetStaticCharField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jchar value)
{
}

static void (JNICALL SetStaticShortField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jshort value)
{
}

static void (JNICALL SetStaticIntField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jint value)
{
}

static void (JNICALL SetStaticLongField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jlong value)
{
}

static void (JNICALL SetStaticFloatField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jfloat value)
{
}

static void (JNICALL SetStaticDoubleField)
        (JNIEnv *env, jclass clazz, jfieldID fieldID, jdouble value)
{
}


static jstring (JNICALL NewString)
        (JNIEnv *env, const jchar *unicode, jsize len)
{
}

static jsize (JNICALL GetStringLength)
        (JNIEnv *env, jstring str)
{
}

static const jchar *(JNICALL GetStringChars)
        (JNIEnv *env, jstring str, jboolean *isCopy)
{
}

static void (JNICALL ReleaseStringChars)
        (JNIEnv *env, jstring str, const jchar *chars)
{
}


static jstring (JNICALL NewStringUTF)
        (JNIEnv *env, const char *utf)
{
}

static jsize (JNICALL GetStringUTFLength)
        (JNIEnv *env, jstring str)
{
}

static const char* (JNICALL GetStringUTFChars)
        (JNIEnv *env, jstring str, jboolean *isCopy)
{
}

static void (JNICALL ReleaseStringUTFChars)
        (JNIEnv *env, jstring str, const char* chars)
{
}



static jsize (JNICALL GetArrayLength)
        (JNIEnv *env, jarray array)
{
}


static jobjectArray (JNICALL NewObjectArray)
        (JNIEnv *env, jsize len, jclass clazz, jobject init)
{
}

static jobject (JNICALL GetObjectArrayElement)
        (JNIEnv *env, jobjectArray array, jsize index)
{
}

static void (JNICALL SetObjectArrayElement)
        (JNIEnv *env, jobjectArray array, jsize index, jobject val)
{
}


static jbooleanArray (JNICALL NewBooleanArray)
        (JNIEnv *env, jsize len)
{
}

static jbyteArray (JNICALL NewByteArray)
        (JNIEnv *env, jsize len)
{
}

static jcharArray (JNICALL NewCharArray)
        (JNIEnv *env, jsize len)
{
}

static jshortArray (JNICALL NewShortArray)
        (JNIEnv *env, jsize len)
{
}

static jintArray (JNICALL NewIntArray)
        (JNIEnv *env, jsize len)
{
}

static jlongArray (JNICALL NewLongArray)
        (JNIEnv *env, jsize len)
{
}

static jfloatArray (JNICALL NewFloatArray)
        (JNIEnv *env, jsize len)
{
}

static jdoubleArray (JNICALL NewDoubleArray)
        (JNIEnv *env, jsize len)
{
}


static jboolean * (JNICALL GetBooleanArrayElements)
        (JNIEnv *env, jbooleanArray array, jboolean *isCopy)
{
}

static jbyte * (JNICALL GetByteArrayElements)
        (JNIEnv *env, jbyteArray array, jboolean *isCopy)
{
}

static jchar * (JNICALL GetCharArrayElements)
        (JNIEnv *env, jcharArray array, jboolean *isCopy)
{
}

static jshort * (JNICALL GetShortArrayElements)
        (JNIEnv *env, jshortArray array, jboolean *isCopy)
{
}

static jint * (JNICALL GetIntArrayElements)
        (JNIEnv *env, jintArray array, jboolean *isCopy)
{
}

static jlong * (JNICALL GetLongArrayElements)
        (JNIEnv *env, jlongArray array, jboolean *isCopy)
{
}

static jfloat * (JNICALL GetFloatArrayElements)
        (JNIEnv *env, jfloatArray array, jboolean *isCopy)
{
}

static jdouble * (JNICALL GetDoubleArrayElements)
        (JNIEnv *env, jdoubleArray array, jboolean *isCopy)
{
}


static void (JNICALL ReleaseBooleanArrayElements)
        (JNIEnv *env, jbooleanArray array, jboolean *elems, jint mode)
{
}

static void (JNICALL ReleaseByteArrayElements)
        (JNIEnv *env, jbyteArray array, jbyte *elems, jint mode)
{
}

static void (JNICALL ReleaseCharArrayElements)
        (JNIEnv *env, jcharArray array, jchar *elems, jint mode)
{
}

static void (JNICALL ReleaseShortArrayElements)
        (JNIEnv *env, jshortArray array, jshort *elems, jint mode)
{
}

static void (JNICALL ReleaseIntArrayElements)
        (JNIEnv *env, jintArray array, jint *elems, jint mode)
{
}

static void (JNICALL ReleaseLongArrayElements)
        (JNIEnv *env, jlongArray array, jlong *elems, jint mode)
{
}

static void (JNICALL ReleaseFloatArrayElements)
        (JNIEnv *env, jfloatArray array, jfloat *elems, jint mode)
{
}

static void (JNICALL ReleaseDoubleArrayElements)
        (JNIEnv *env, jdoubleArray array, jdouble *elems, jint mode)
{
}


static void (JNICALL GetBooleanArrayRegion)
        (JNIEnv *env, jbooleanArray array, jsize start, jsize l, jboolean *buf)
{
}

static void (JNICALL GetByteArrayRegion)
        (JNIEnv *env, jbyteArray array, jsize start, jsize len, jbyte *buf)
{
}

static void (JNICALL GetCharArrayRegion)
        (JNIEnv *env, jcharArray array, jsize start, jsize len, jchar *buf)
{
}

static void (JNICALL GetShortArrayRegion)
        (JNIEnv *env, jshortArray array, jsize start, jsize len, jshort *buf)
{
}

static void (JNICALL GetIntArrayRegion)
        (JNIEnv *env, jintArray array, jsize start, jsize len, jint *buf)
{
}

static void (JNICALL GetLongArrayRegion)
        (JNIEnv *env, jlongArray array, jsize start, jsize len, jlong *buf)
{
}

static void (JNICALL GetFloatArrayRegion)
        (JNIEnv *env, jfloatArray array, jsize start, jsize len, jfloat *buf)
{
}

static void (JNICALL GetDoubleArrayRegion)
        (JNIEnv *env, jdoubleArray array, jsize start, jsize len, jdouble *buf)
{
}


static void (JNICALL SetBooleanArrayRegion)
        (JNIEnv *env, jbooleanArray array, jsize start, jsize l, const jboolean *buf)
{
}

static void (JNICALL SetByteArrayRegion)
        (JNIEnv *env, jbyteArray array, jsize start, jsize len, const jbyte *buf)
{
}

static void (JNICALL SetCharArrayRegion)
        (JNIEnv *env, jcharArray array, jsize start, jsize len, const jchar *buf)
{
}

static void (JNICALL SetShortArrayRegion)
        (JNIEnv *env, jshortArray array, jsize start, jsize len, const jshort *buf)
{
}

static void (JNICALL SetIntArrayRegion)
        (JNIEnv *env, jintArray array, jsize start, jsize len, const jint *buf)
{
}

static void (JNICALL SetLongArrayRegion)
        (JNIEnv *env, jlongArray array, jsize start, jsize len, const jlong *buf)
{
}

static void (JNICALL SetFloatArrayRegion)
        (JNIEnv *env, jfloatArray array, jsize start, jsize len, const jfloat *buf)
{
}

static void (JNICALL SetDoubleArrayRegion)
        (JNIEnv *env, jdoubleArray array, jsize start, jsize len, const jdouble *buf)
{
}


static jint (JNICALL RegisterNatives)
        (JNIEnv *env, jclass clazz, const JNINativeMethod *methods,
         jint nMethods)
{
}

static jint (JNICALL UnregisterNatives)
        (JNIEnv *env, jclass clazz)
{
}


static jint (JNICALL MonitorEnter)
        (JNIEnv *env, jobject obj)
{
}

static jint (JNICALL MonitorExit)
        (JNIEnv *env, jobject obj)
{
}


static jint (JNICALL GetJavaVM)
        (JNIEnv *env, JavaVM **vm)
{
}


static void (JNICALL GetStringRegion)
        (JNIEnv *env, jstring str, jsize start, jsize len, jchar *buf)
{
}

static void (JNICALL GetStringUTFRegion)
        (JNIEnv *env, jstring str, jsize start, jsize len, char *buf)
{
}


static void * (JNICALL GetPrimitiveArrayCritical)
        (JNIEnv *env, jarray array, jboolean *isCopy)
{
}

static void (JNICALL ReleasePrimitiveArrayCritical)
        (JNIEnv *env, jarray array, void *carray, jint mode)
{
}


static const jchar * (JNICALL GetStringCritical)
        (JNIEnv *env, jstring string, jboolean *isCopy)
{
}

static void (JNICALL ReleaseStringCritical)
        (JNIEnv *env, jstring string, const jchar *cstring)
{
}


static jweak (JNICALL NewWeakGlobalRef)
        (JNIEnv *env, jobject obj)
{
}

static void (JNICALL DeleteWeakGlobalRef)
        (JNIEnv *env, jweak ref)
{
}


static jboolean (JNICALL ExceptionCheck)
        (JNIEnv *env)
{
}


static jobject (JNICALL NewDirectByteBuffer)
        (JNIEnv* env, void* address, jlong capacity)
{
}

static void* (JNICALL GetDirectBufferAddress)
        (JNIEnv* env, jobject buf)
{
}

static jlong (JNICALL GetDirectBufferCapacity)
        (JNIEnv* env, jobject buf)
{
}


/* New JNI 1.6 Features */

static jobjectRefType (JNICALL GetObjectRefType)
        (JNIEnv* env, jobject obj)
{

}





static JNINativeInterface_ g_native_interface = {
        .reserved0 = nullptr,
        .reserved1 = nullptr,
        .reserved2 = nullptr,
        .reserved3 = nullptr,
        .GetVersion = ::GetVersion,
        .DefineClass = ::DefineClass,
        .FindClass = ::FindClass,
        .FromReflectedMethod = ::FromReflectedMethod,
        .FromReflectedField = ::FromReflectedField,
        .ToReflectedMethod = ::ToReflectedMethod,
        .GetSuperclass = ::GetSuperclass,
        .IsAssignableFrom = ::IsAssignableFrom,
        .ToReflectedField = ::ToReflectedField,

        .Throw = ::Throw,
        .ThrowNew = ::ThrowNew,
        .ExceptionOccurred = ::ExceptionOccurred,
        .ExceptionDescribe = ::ExceptionDescribe,
        .ExceptionClear = ::ExceptionClear,
        .FatalError = ::FatalError,

        .PushLocalFrame = ::PushLocalFrame,
        .PopLocalFrame = ::PopLocalFrame,

        .NewGlobalRef = ::NewGlobalRef,
        .DeleteGlobalRef = ::DeleteGlobalRef,
        .DeleteLocalRef = ::DeleteLocalRef,
        .IsSameObject = ::IsSameObject,
        .NewLocalRef = ::NewLocalRef,
        .EnsureLocalCapacity = ::EnsureLocalCapacity,

        .AllocObject = ::AllocObject,
        .NewObject = ::NewObject,
        .NewObjectV = ::NewObjectV,
        .NewObjectA = ::NewObjectA,

        .GetObjectClass = ::GetObjectClass,
        .IsInstanceOf = ::IsInstanceOf,

        .GetMethodID = ::GetMethodID,

        .CallObjectMethod = ::CallObjectMethod,
        .CallObjectMethodV = ::CallObjectMethodV,
        .CallObjectMethodA = ::CallObjectMethodA,

        .CallBooleanMethod = ::CallBooleanMethod,
        .CallBooleanMethodV = ::CallBooleanMethodV,
        .CallBooleanMethodA = ::CallBooleanMethodA,

        .CallByteMethod = ::CallByteMethod,
        .CallByteMethodV = ::CallByteMethodV,
        .CallByteMethodA = ::CallByteMethodA,

        .CallCharMethod = ::CallCharMethod,
        .CallCharMethodV = ::CallCharMethodV,
        .CallCharMethodA = ::CallCharMethodA,

        .CallShortMethod = ::CallShortMethod,
        .CallShortMethodV = ::CallShortMethodV,
        .CallShortMethodA = ::CallShortMethodA,

        .CallIntMethod = ::CallIntMethod,
        .CallIntMethodV = ::CallIntMethodV,
        .CallIntMethodA = ::CallIntMethodA,

        .CallLongMethod = ::CallLongMethod,
        .CallLongMethodV = ::CallLongMethodV,
        .CallLongMethodA = ::CallLongMethodA,

        .CallFloatMethod = ::CallFloatMethod,
        .CallFloatMethodV = ::CallFloatMethodV,
        .CallFloatMethodA = ::CallFloatMethodA,

        .CallDoubleMethod = ::CallDoubleMethod,
        .CallDoubleMethodV = ::CallDoubleMethodV,
        .CallDoubleMethodA = ::CallDoubleMethodA,

        .CallVoidMethod = ::CallVoidMethod,
        .CallVoidMethodV = ::CallVoidMethodV,
        .CallVoidMethodA = ::CallVoidMethodA,

        .CallNonvirtualObjectMethod = ::CallNonvirtualObjectMethod,
        .CallNonvirtualObjectMethodV = ::CallNonvirtualObjectMethodV,
        .CallNonvirtualObjectMethodA = ::CallNonvirtualObjectMethodA,

        .CallNonvirtualBooleanMethod = ::CallNonvirtualBooleanMethod,
        .CallNonvirtualBooleanMethodV = ::CallNonvirtualBooleanMethodV,
        .CallNonvirtualBooleanMethodA = ::CallNonvirtualBooleanMethodA,

        .CallNonvirtualByteMethod = ::CallNonvirtualByteMethod,
        .CallNonvirtualByteMethodV = ::CallNonvirtualByteMethodV,
        .CallNonvirtualByteMethodA = ::CallNonvirtualByteMethodA,

        .CallNonvirtualCharMethod = ::CallNonvirtualCharMethod,
        .CallNonvirtualCharMethodV = ::CallNonvirtualCharMethodV,
        .CallNonvirtualCharMethodA = ::CallNonvirtualCharMethodA,

        .CallNonvirtualShortMethod = ::CallNonvirtualShortMethod,
        .CallNonvirtualShortMethodV = ::CallNonvirtualShortMethodV,
        .CallNonvirtualShortMethodA = ::CallNonvirtualShortMethodA,

        .CallNonvirtualIntMethod = ::CallNonvirtualIntMethod,
        .CallNonvirtualIntMethodV = ::CallNonvirtualIntMethodV,
        .CallNonvirtualIntMethodA = ::CallNonvirtualIntMethodA,

        .CallNonvirtualLongMethod = ::CallNonvirtualLongMethod,
        .CallNonvirtualLongMethodV = ::CallNonvirtualLongMethodV,
        .CallNonvirtualLongMethodA = ::CallNonvirtualLongMethodA,

        .CallNonvirtualFloatMethod = ::CallNonvirtualFloatMethod,
        .CallNonvirtualFloatMethodV = ::CallNonvirtualFloatMethodV,
        .CallNonvirtualFloatMethodA = ::CallNonvirtualFloatMethodA,

        .CallNonvirtualDoubleMethod = ::CallNonvirtualDoubleMethod,
        .CallNonvirtualDoubleMethodV = ::CallNonvirtualDoubleMethodV,
        .CallNonvirtualDoubleMethodA = ::CallNonvirtualDoubleMethodA,

        .CallNonvirtualVoidMethod = ::CallNonvirtualVoidMethod,
        .CallNonvirtualVoidMethodV = ::CallNonvirtualVoidMethodV,
        .CallNonvirtualVoidMethodA = ::CallNonvirtualVoidMethodA,


        .GetFieldID = ::GetFieldID,
        .GetObjectField = ::GetObjectField,
        .GetBooleanField = ::GetBooleanField,
        .GetByteField = ::GetByteField,
        .GetCharField = ::GetCharField,
        .GetShortField = ::GetShortField,
        .GetIntField = ::GetIntField,
        .GetLongField = ::GetLongField,
        .GetFloatField = ::GetFloatField,
        .GetDoubleField = ::GetDoubleField,

        .SetObjectField = ::SetObjectField,
        .SetBooleanField = ::SetBooleanField,
        .SetByteField = ::SetByteField,
        .SetCharField = ::SetCharField,
        .SetShortField = ::SetShortField,
        .SetIntField = ::SetIntField,
        .SetLongField = ::SetLongField,
        .SetFloatField = ::SetFloatField,
        .SetDoubleField = ::SetDoubleField,

        .GetStaticMethodID = ::GetStaticMethodID,

        .CallStaticObjectMethod = ::CallStaticObjectMethod,
        .CallStaticObjectMethodV = ::CallStaticObjectMethodV,
        .CallStaticObjectMethodA = ::CallStaticObjectMethodA,

        .CallStaticBooleanMethod = ::CallStaticBooleanMethod,
        .CallStaticBooleanMethodV = ::CallStaticBooleanMethodV,
        .CallStaticBooleanMethodA = ::CallStaticBooleanMethodA,

        .CallStaticByteMethod = ::CallStaticByteMethod,
        .CallStaticByteMethodV = ::CallStaticByteMethodV,
        .CallStaticByteMethodA = ::CallStaticByteMethodA,

        .CallStaticCharMethod = ::CallStaticCharMethod,
        .CallStaticCharMethodV = ::CallStaticCharMethodV,
        .CallStaticCharMethodA = ::CallStaticCharMethodA,

        .CallStaticShortMethod = ::CallStaticShortMethod,
        .CallStaticShortMethodV = ::CallStaticShortMethodV,
        .CallStaticShortMethodA = ::CallStaticShortMethodA,


        .CallStaticIntMethod = ::CallStaticIntMethod,
        .CallStaticIntMethodV = ::CallStaticIntMethodV,
        .CallStaticIntMethodA = ::CallStaticIntMethodA,

        .CallStaticLongMethod = ::CallStaticLongMethod,
        .CallStaticLongMethodV = ::CallStaticLongMethodV,
        .CallStaticLongMethodA = ::CallStaticLongMethodA,

        .CallStaticFloatMethod = ::CallStaticFloatMethod,
        .CallStaticFloatMethodV = ::CallStaticFloatMethodV,
        .CallStaticFloatMethodA = ::CallStaticFloatMethodA,

        .CallStaticDoubleMethod = ::CallStaticDoubleMethod,
        .CallStaticDoubleMethodV = ::CallStaticDoubleMethodV,
        .CallStaticDoubleMethodA = ::CallStaticDoubleMethodA,

        .CallStaticVoidMethod = CallStaticVoidMethod,
        .CallStaticVoidMethodV = CallStaticVoidMethodV,
        .CallStaticVoidMethodA = CallStaticVoidMethodA,

        .GetStaticFieldID = ::GetStaticFieldID,
        .GetStaticObjectField = ::GetStaticObjectField,
        .GetStaticBooleanField = ::GetStaticBooleanField,
        .GetStaticByteField = ::GetStaticByteField,
        .GetStaticCharField = ::GetStaticCharField,
        .GetStaticShortField = ::GetStaticShortField,
        .GetStaticIntField = ::GetStaticIntField,
        .GetStaticLongField = ::GetStaticLongField,
        .GetStaticFloatField = ::GetStaticFloatField,
        .GetStaticDoubleField = ::GetStaticDoubleField,

        .SetStaticObjectField = ::SetStaticObjectField,
        .SetStaticBooleanField = ::SetStaticBooleanField,
        .SetStaticByteField = ::SetStaticByteField,
        .SetStaticCharField = ::SetStaticCharField,
        .SetStaticShortField = ::SetStaticShortField,
        .SetStaticIntField = ::SetStaticIntField,
        .SetStaticLongField = ::SetStaticLongField,
        .SetStaticFloatField = ::SetStaticFloatField,
        .SetStaticDoubleField = ::SetStaticDoubleField,

        .NewString = ::NewString,
        .GetStringLength = ::GetStringLength,
        .GetStringChars = ::GetStringChars,
        .ReleaseStringChars = ::ReleaseStringChars,

        .NewStringUTF = ::NewStringUTF,
        .GetStringUTFLength = ::GetStringUTFLength,
        .GetStringUTFChars = ::GetStringUTFChars,
        .ReleaseStringUTFChars = ::ReleaseStringUTFChars,

        .GetArrayLength = ::GetArrayLength,

        .NewObjectArray = ::NewObjectArray,
        .GetObjectArrayElement = ::GetObjectArrayElement,
        .SetObjectArrayElement = ::SetObjectArrayElement,

        .NewBooleanArray = ::NewBooleanArray,
        .NewByteArray = ::NewByteArray,
        .NewCharArray = ::NewCharArray,
        .NewShortArray = ::NewShortArray,
        .NewIntArray = ::NewIntArray,
        .NewLongArray = ::NewLongArray,
        .NewFloatArray = ::NewFloatArray,
        .NewDoubleArray = ::NewDoubleArray,

        .GetBooleanArrayElements = ::GetBooleanArrayElements,
        .GetByteArrayElements = ::GetByteArrayElements,
        .GetCharArrayElements = ::GetCharArrayElements,
        .GetShortArrayElements = ::GetShortArrayElements,
        .GetIntArrayElements = ::GetIntArrayElements,
        .GetLongArrayElements = ::GetLongArrayElements,
        .GetFloatArrayElements = ::GetFloatArrayElements,
        .GetDoubleArrayElements = ::GetDoubleArrayElements,

        .ReleaseBooleanArrayElements = ::ReleaseBooleanArrayElements,
        .ReleaseByteArrayElements = ::ReleaseByteArrayElements,
        .ReleaseCharArrayElements = ::ReleaseCharArrayElements,
        .ReleaseShortArrayElements = ::ReleaseShortArrayElements,
        .ReleaseIntArrayElements = ::ReleaseIntArrayElements,
        .ReleaseLongArrayElements = ::ReleaseLongArrayElements,
        .ReleaseFloatArrayElements = ::ReleaseFloatArrayElements,
        .ReleaseDoubleArrayElements = ::ReleaseDoubleArrayElements,

        .GetBooleanArrayRegion = ::GetBooleanArrayRegion,
        .GetByteArrayRegion = ::GetByteArrayRegion,
        .GetCharArrayRegion = ::GetCharArrayRegion,
        .GetShortArrayRegion = ::GetShortArrayRegion,
        .GetIntArrayRegion = ::GetIntArrayRegion,
        .GetLongArrayRegion = ::GetLongArrayRegion,
        .GetFloatArrayRegion = ::GetFloatArrayRegion,
        .GetDoubleArrayRegion = ::GetDoubleArrayRegion,

        .SetBooleanArrayRegion = ::SetBooleanArrayRegion,
        .SetByteArrayRegion = ::SetByteArrayRegion,
        .SetCharArrayRegion = ::SetCharArrayRegion,
        .SetShortArrayRegion = ::SetShortArrayRegion,
        .SetIntArrayRegion = ::SetIntArrayRegion,
        .SetLongArrayRegion = ::SetLongArrayRegion,
        .SetFloatArrayRegion = ::SetFloatArrayRegion,
        .SetDoubleArrayRegion = ::SetDoubleArrayRegion,

        .RegisterNatives = ::RegisterNatives,
        .UnregisterNatives = ::UnregisterNatives,

        .MonitorEnter = ::MonitorEnter,
        .MonitorExit = ::MonitorExit,

        .GetJavaVM = ::GetJavaVM,

        .GetStringRegion = ::GetStringRegion,
        .GetStringUTFRegion = ::GetStringUTFRegion,

        .GetPrimitiveArrayCritical = ::GetPrimitiveArrayCritical,
        .ReleasePrimitiveArrayCritical = ::ReleasePrimitiveArrayCritical,

        .GetStringCritical = ::GetStringCritical,
        .ReleaseStringCritical = ::ReleaseStringCritical,

        .NewWeakGlobalRef = ::NewWeakGlobalRef,
        .DeleteWeakGlobalRef = ::DeleteWeakGlobalRef,

        .ExceptionCheck = ::ExceptionCheck,

        .NewDirectByteBuffer = ::NewDirectByteBuffer,
        .GetDirectBufferAddress = ::GetDirectBufferAddress,
        .GetDirectBufferCapacity = ::GetDirectBufferCapacity,


        /* New JNI 1.6 Features */
        .GetObjectRefType = ::GetObjectRefType,
};
