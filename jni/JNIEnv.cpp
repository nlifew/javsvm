

#include <cstdarg>

#include "jni.h"
#include "jni_utils.h"


#include "../object/jfield.h"
#include "../utils/strings.h"

#include "access_field.h"
#include "access_array.h"
#include "call_method.h"
#include "string_utf.h"
#include "reference.h"
#include "nio.h"
#include "reflection.h"


struct _jfieldID
{
};

struct _jmethodID
{
};



static jint (JNICALL GetVersion)
        (JNIEnv *)
{
    // 只支持常量，jni 1.8 接口
    return JNI_VERSION_1_8;
}


static jclass (JNICALL DefineClass)
        (JNIEnv *env, const char *name, jobject loader, const jbyte *buf,
         jsize len)
{
    // 不支持运行时加载类
    return nullptr;
}

static jclass (JNICALL FindClass)
        (JNIEnv *, const char *name)
{
    if (name == nullptr) {
        return nullptr;
    }
    auto _clazz = javsvm::jclass::load_class(name);
    if (_clazz == nullptr) {
        return nullptr;
    }
    return (jclass) to_object(_clazz->object);
}


static jclass (JNICALL GetSuperclass)
        (JNIEnv *, jclass sub)
{
    auto _clazz = to_class(sub);
    if (_clazz == nullptr) {
        return nullptr;
    }
    if (HAS_FLAG(_clazz->access_flag, javsvm::jclass_file::ACC_INTERFACE)) {
        // 接口类始终返回 nullptr
        return nullptr;
    }
    return (jclass) to_object(_clazz->super_class->object);
}

static jboolean (JNICALL IsAssignableFrom)
        (JNIEnv *, jclass sub, jclass sup)
{
    auto _sub = to_class(sub);
    auto _sup = to_class(sup);

    if (_sup == nullptr) {
        return JNI_FALSE;
    }
    return _sup->is_assign_from(_sub);
}



static jint (JNICALL Throw)
        (JNIEnv *, jthrowable obj)
{
    javsvm::throw_throwable(to_object(obj));
    return 0;
}

static jint (JNICALL ThrowNew)
        (JNIEnv *env, jclass clazz, const char *msg)
{
    jmethodID _init_ = env->GetMethodID(clazz, "<init>", "(Ljava/lang/String;)V");
    jstring _msg = env->NewStringUTF(msg);

    if (_init_ == nullptr || _msg == nullptr) {
        return -1;
    }
    auto exp = (jthrowable) env->NewObject(clazz, _init_);
    if (exp == nullptr) {
        return -1;
    }
    env->Throw(exp);
    return 0;
}

static jthrowable (JNICALL ExceptionOccurred)
        (JNIEnv *)
{
    auto _obj = javsvm::check_exception();
    return (jthrowable) to_object(_obj);
}

static void (JNICALL ExceptionDescribe)
        (JNIEnv *)
{
    // todo
}

static void (JNICALL ExceptionClear)
        (JNIEnv *)
{
    javsvm::clear_exception();
}

static void (JNICALL FatalError)
        (JNIEnv *, const char *msg)
{
    javsvm::throw_err("java/lang/Error", msg == nullptr ? "" : msg);
}


static jint (JNICALL PushLocalFrame)
        (JNIEnv *env, jint capacity)
{
    // todo
    return 0;
}

static jobject (JNICALL PopLocalFrame)
        (JNIEnv *env, jobject result)
{
    // todo
    return nullptr;
}




static jobject (JNICALL AllocObject)
        (JNIEnv *, jclass clazz)
{
    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return nullptr;
    }
    return to_object(_clazz->new_instance());
}



static jobject (JNICALL NewObjectA)
        (JNIEnv *, jclass clazz, jmethodID methodID, const jvalue *j_args)
{
    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return nullptr;
    }
    auto _method = (javsvm::jmethod *) methodID;
    if (_method == nullptr) {
        return nullptr;
    }
    auto _args = to_args(methodID, j_args);
    javsvm::jargs args(_args.get());

    auto _ret = _clazz->new_instance();
    _method->invoke_special(_ret, args);

    return to_object(_ret);
}


static jobject (JNICALL NewObjectV)
        (JNIEnv *, jclass clazz, jmethodID methodID, va_list v_args)
{
    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return nullptr;
    }
    auto _method = (javsvm::jmethod *) methodID;
    if (_method == nullptr) {
        return nullptr;
    }
    auto _args = to_args(methodID, v_args);
    javsvm::jargs args(_args.get());

    auto _ret = _clazz->new_instance();
    _method->invoke_special(_ret, args);

    return to_object(_ret);
}


static jobject (JNICALL NewObject)
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...)
{
    va_list ap;
    va_start(ap, methodID);
    auto ret = NewObjectV(env, clazz, methodID, ap);
    va_end(ap);
    return ret;
}


static jclass (JNICALL GetObjectClass)
        (JNIEnv *, jobject obj)
{
    auto _obj = to_object(obj);
    auto _obj_ptr = javsvm::jvm::get().heap.lock(_obj);
    if (_obj_ptr == nullptr) {
        return nullptr;
    }
    return (jclass) to_object(_obj_ptr->klass->object);
}

static jboolean (JNICALL IsInstanceOf)
        (JNIEnv *, jobject obj, jclass clazz)
{
    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return JNI_FALSE;
    }
    return _clazz->is_instance(to_object(obj));
}


static jint (JNICALL RegisterNatives)
        (JNIEnv *env, jclass clazz, const JNINativeMethod *methods,
         jint nMethods)
{
    auto _clazz = to_class(clazz);
    LOGI("RegisterNatives: %s\n", _clazz == nullptr ? "" : _clazz->name);

    if (_clazz == nullptr) {
        return -1;
    }
    auto result = 0;

    for (int i = 0; i < nMethods; ++i) {
        LOGI("RegisterNatives: [%d/%d] name='%s', 'sig'='%s'\n",
             i, nMethods, methods[i].name, methods[i].signature);

        auto _method = _clazz->get_method(methods[i].name, methods[i].signature);
        if (_method == nullptr) {
            LOGE("RegisterNatives: miss method, ignore\n");
            result = -1;
            continue;
        }
        if (! HAS_FLAG(_method->access_flag, javsvm::jclass_method::ACC_NATIVE)) {
            LOGE("RegisterNatives: target method is NOT native, throw exception\n");

            auto exp_class = env->FindClass("java/lang/NoSuchMethodError");
            char buff[512];
            snprintf(buff, sizeof(buff), "failed to register native method '%s' -> %s%s",
                     _clazz->name, methods[i].name, methods[i].signature);
            env->ThrowNew(exp_class, "%s");
            continue;
        }
        if (_method->entrance.jni_func != nullptr) {
            LOGW("RegisterNatives: multi native method function to java method, the previous is: %p\n",
                 _method->entrance.jni_func);
        }
        _method->entrance.jni_func = methods[i].fnPtr;
    }
    return result;
}

static jint (JNICALL UnregisterNatives)
        (JNIEnv *env, jclass clazz)
{
    auto _clazz = to_class(clazz);
    LOGI("UnregisterNatives: %s\n", _clazz == nullptr ? "" : _clazz->name);

    if (_clazz == nullptr) {
        return -1;
    }
    for (int i = 0, z = _clazz->method_table_size; i < z; i ++) {
        auto &method = _clazz->method_tables[i];
        LOGI("UnregisterNatives: [%d/%d] '%s'%s\n", i, z, method.name, method.sig);

        if (HAS_FLAG(method.access_flag, javsvm::jclass_method::ACC_NATIVE)) {
            method.entrance.jni_func = nullptr;
        }
    }
    return 0;
}


static jint (JNICALL MonitorEnter)
        (JNIEnv *env, jobject obj)
{
    auto ptr = javsvm::jvm::get().heap.lock(to_object(obj));
    if (ptr == nullptr) {
        javsvm::throw_exp("java/lang/NullPointerException", "monitorenter");
        return -1;
    }
    return ptr->lock();
}

static jint (JNICALL MonitorExit)
        (JNIEnv *env, jobject obj)
{
    auto ptr = javsvm::jvm::get().heap.lock(to_object(obj));
    if (ptr == nullptr) {
        javsvm::throw_exp("java/lang/NullPointerException", "monitorexit");
        return -1;
    }
    return ptr->unlock();
}


static jint (JNICALL GetJavaVM)
        (JNIEnv *env, JavaVM **vm)
{
    if (vm) *vm = (JavaVM *) env->functions->reserved1;
    return 0;
}


static jboolean (JNICALL IsSameObject)
        (JNIEnv *, jobject obj1, jobject obj2)
{
    auto &jvm = javsvm::jvm::get();
    auto _obj1 = jvm.heap.lock(to_object(obj1));
    auto _obj2 = jvm.heap.lock(to_object(obj2));

    return _obj1 == _obj2;
}





static jboolean (JNICALL ExceptionCheck)
        (JNIEnv *env)
{
    return ExceptionOccurred(env) != nullptr;
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

int init_jni_env(JNINativeInterface_ *dst, javsvm::jenv *env) noexcept
{
    memcpy(dst, &g_native_interface, sizeof(JNIInvokeInterface_));
    dst->reserved0 = env;
    dst->reserved1 = env->jvm.jni();
    return 0;
}