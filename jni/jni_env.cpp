

#include "jni_env.h"
#include "jni_utils.h"

struct _jfieldID
{
};

struct _jmethodID
{
};

namespace jni
{
jint (JNICALL GetVersion)
        (JNIEnv *) {
    // 只支持常量，jni 1.8 接口
    return JNI_VERSION_1_8;
}


jclass (JNICALL DefineClass)
        (JNIEnv *env, const char *name, jobject loader, const jbyte *buf,
         jsize len) {
    // todo 不支持运行时加载类
    return nullptr;
}

jclass (JNICALL FindClass)
        (JNIEnv *, const char *name) {
    safety_area_guard guard;

    auto _clazz = javsvm::jclass::load_class(name);
    if (_clazz == nullptr) {
        return nullptr;
    }
    return (jclass) to_object(_clazz->object.get());
}


jclass (JNICALL GetSuperclass)
        (JNIEnv *, jclass sub) {
    safety_area_guard guard;

    auto _clazz = to_class(sub);
    if (_clazz == nullptr) {
        return nullptr;
    }
    if (HAS_FLAG(_clazz->access_flag, javsvm::jclass_file::ACC_INTERFACE)) {
        // 接口类始终返回 nullptr
        return nullptr;
    }
    return (jclass) to_object(_clazz->super_class->object.get());
}

jboolean (JNICALL IsAssignableFrom)
        (JNIEnv *, jclass sub, jclass sup) {
    javsvm::jclass *_sub;
    javsvm::jclass *_sup;
    {
        safety_area_guard guard;
        _sub = to_class(sub);
        _sup = to_class(sup);
    }

    if (_sup == nullptr) {
        return JNI_FALSE;
    }
    return _sup->is_assign_from(_sub);
}


jint (JNICALL Throw)
        (JNIEnv *, jthrowable obj) {
    safety_area_guard guard;
    javsvm::throw_throwable(to_object(obj));
    return 0;
}

jint (JNICALL ThrowNew)
        (JNIEnv *env, jclass clazz, const char *msg) {
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

jthrowable (JNICALL ExceptionOccurred)
        (JNIEnv *) {
    safety_area_guard guard;
    auto _obj = javsvm::check_exception();
    return (jthrowable) to_object(_obj);
}

void (JNICALL ExceptionDescribe)
        (JNIEnv *) {
    // todo
}

void (JNICALL ExceptionClear)
        (JNIEnv *) {
    safety_area_guard guard;
    javsvm::clear_exception();
}

void (JNICALL FatalError)
        (JNIEnv *, const char *msg) {
    safety_area_guard guard;
    javsvm::throw_err("java/lang/Error", msg == nullptr ? "" : msg);
}


jint (JNICALL PushLocalFrame)
        (JNIEnv *, jint capacity) {
    auto frame = (javsvm::jni_stack_frame *) javsvm::jvm::get().env().stack.top();
    frame->reserve(capacity + frame->local_ref_table_size);
    return 0;
}

jobject (JNICALL PopLocalFrame)
        (JNIEnv *, jobject result) {
    return result;
}


jobject (JNICALL AllocObject)
        (JNIEnv *, jclass clazz) {
    safety_area_guard guard;

    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return nullptr;
    }
    return to_object(_clazz->new_instance());
}


jobject (JNICALL NewObjectA)
        (JNIEnv *, jclass clazz, jmethodID methodID, const jvalue *j_args) {
    safety_area_guard guard;

    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return nullptr;
    }
    auto _method = (javsvm::jmethod *) methodID;
    if (_method == nullptr) {
        return nullptr;
    }

    // 如果是抽象类，抛出异常
    if (HAS_FLAG(_method->access_flag, javsvm::jclass_method::ACC_ABSTRACT)) {
        javsvm::throw_exp("java/lang/InstantiationException", "");
        return nullptr;
    }

    auto _ret = _clazz->new_instance();
    auto ret = to_object(_ret);

    auto _args = to_args(methodID, ret, j_args);
    javsvm::jargs args(_args.get());

    _method->invoke_special(_ret, args);
    return ret;
}


jobject (JNICALL NewObjectV)
        (JNIEnv *, jclass clazz, jmethodID methodID, va_list v_args) {
    safety_area_guard guard;

    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return nullptr;
    }

    if (HAS_FLAG(_clazz->access_flag, javsvm::jclass_file::ACC_ABSTRACT)) {
        javsvm::throw_exp("java/lang/InstantiationError", "");
        return nullptr;
    }

    auto _method = (javsvm::jmethod *) methodID;
    if (_method == nullptr) {
        return nullptr;
    }

    auto _ret = _clazz->new_instance(_method, v_args);
    return to_object(_ret);
}


jobject (JNICALL NewObject)
        (JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    va_list ap;
    va_start(ap, methodID);
    auto ret = NewObjectV(env, clazz, methodID, ap);
    va_end(ap);
    return ret;
}


jclass (JNICALL GetObjectClass)
        (JNIEnv *, jobject obj) {
    safety_area_guard guard;

    auto _obj_ptr = javsvm::jheap::cast(to_object(obj));
    if (_obj_ptr == nullptr) {
        return nullptr;
    }
    return to_object<jclass>(_obj_ptr->klass->object.get());
}

jboolean (JNICALL IsInstanceOf)
        (JNIEnv *, jobject obj, jclass clazz) {
    safety_area_guard guard;

    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return JNI_FALSE;
    }
    return _clazz->is_instance(to_object(obj));
}


jint (JNICALL RegisterNatives)
        (JNIEnv *, jclass clazz, const JNINativeMethod *methods,
         jint nMethods) {
    safety_area_guard guard;

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
        if (!HAS_FLAG(_method->access_flag, javsvm::jclass_method::ACC_NATIVE)) {
            LOGE("RegisterNatives: target method is NOT native, throw exception\n");

            char buff[512];
            snprintf(buff, sizeof(buff), "failed to register native method '%s' -> %s%s",
                     _clazz->name, methods[i].name, methods[i].signature);
            javsvm::throw_exp("java/lang/NoSuchMethodError", buff);
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

jint (JNICALL UnregisterNatives)
        (JNIEnv *, jclass clazz) {
    safety_area_guard guard;

    auto _clazz = to_class(clazz);
    LOGI("UnregisterNatives: %s\n", _clazz == nullptr ? "" : _clazz->name);

    if (_clazz == nullptr) {
        return -1;
    }
    for (int i = 0, z = _clazz->method_table_size; i < z; i++) {
        auto &method = _clazz->method_tables[i];
        LOGI("UnregisterNatives: [%d/%d] '%s'%s\n", i, z, method.name, method.sig);

        if (HAS_FLAG(method.access_flag, javsvm::jclass_method::ACC_NATIVE)) {
            method.entrance.jni_func = nullptr;
        }
    }
    return 0;
}


jint (JNICALL MonitorEnter)
        (JNIEnv *, jobject obj) {

    javsvm::leave_safety_area();

    auto _obj = javsvm::jheap::cast(to_object(obj));
    if (_obj == nullptr) {
        javsvm::enter_safety_area();
        return -1;
    }
    auto lock_event = _obj->lock_internal();

    std::atomic_thread_fence(std::memory_order_seq_cst);

    javsvm::enter_safety_area();
    lock_event->lock();

    return 0;
}

jint (JNICALL MonitorExit)
        (JNIEnv *, jobject obj) {

    safety_area_guard guard;

    auto _obj = javsvm::jheap::cast(to_object(obj));
    if (_obj == nullptr) {
        return -1;
    }
    return _obj->unlock();
}


jint (JNICALL GetJavaVM)
        (JNIEnv *, JavaVM **vm) {
    if (vm) *vm = java_vm;
    return 0;
}


jboolean (JNICALL ExceptionCheck)
        (JNIEnv *env) {
    return ExceptionOccurred(env) != nullptr;
}
} // namespace jni

JNINativeInterface_ s_jni_native_interface = {
        .reserved0 = nullptr,
        .reserved1 = nullptr,
        .reserved2 = nullptr,
        .reserved3 = nullptr,
        .GetVersion = jni::GetVersion,
        .DefineClass = jni::DefineClass,
        .FindClass = jni::FindClass,
        .FromReflectedMethod = jni::FromReflectedMethod,
        .FromReflectedField = jni::FromReflectedField,
        .ToReflectedMethod = jni::ToReflectedMethod,
        .GetSuperclass = jni::GetSuperclass,
        .IsAssignableFrom = jni::IsAssignableFrom,
        .ToReflectedField = jni::ToReflectedField,

        .Throw = jni::Throw,
        .ThrowNew = jni::ThrowNew,
        .ExceptionOccurred = jni::ExceptionOccurred,
        .ExceptionDescribe = jni::ExceptionDescribe,
        .ExceptionClear = jni::ExceptionClear,
        .FatalError = jni::FatalError,

        .PushLocalFrame = jni::PushLocalFrame,
        .PopLocalFrame = jni::PopLocalFrame,

        .NewGlobalRef = jni::NewGlobalRef,
        .DeleteGlobalRef = jni::DeleteGlobalRef,
        .DeleteLocalRef = jni::DeleteLocalRef,
        .IsSameObject = jni::IsSameObject,
        .NewLocalRef = jni::NewLocalRef,
        .EnsureLocalCapacity = jni::EnsureLocalCapacity,

        .AllocObject = jni::AllocObject,
        .NewObject = jni::NewObject,
        .NewObjectV = jni::NewObjectV,
        .NewObjectA = jni::NewObjectA,

        .GetObjectClass = jni::GetObjectClass,
        .IsInstanceOf = jni::IsInstanceOf,

        .GetMethodID = jni::GetMethodID,

        .CallObjectMethod = jni::CallObjectMethod,
        .CallObjectMethodV = jni::CallObjectMethodV,
        .CallObjectMethodA = jni::CallObjectMethodA,

        .CallBooleanMethod = jni::CallBooleanMethod,
        .CallBooleanMethodV = jni::CallBooleanMethodV,
        .CallBooleanMethodA = jni::CallBooleanMethodA,

        .CallByteMethod = jni::CallByteMethod,
        .CallByteMethodV = jni::CallByteMethodV,
        .CallByteMethodA = jni::CallByteMethodA,

        .CallCharMethod = jni::CallCharMethod,
        .CallCharMethodV = jni::CallCharMethodV,
        .CallCharMethodA = jni::CallCharMethodA,

        .CallShortMethod = jni::CallShortMethod,
        .CallShortMethodV = jni::CallShortMethodV,
        .CallShortMethodA = jni::CallShortMethodA,

        .CallIntMethod = jni::CallIntMethod,
        .CallIntMethodV = jni::CallIntMethodV,
        .CallIntMethodA = jni::CallIntMethodA,

        .CallLongMethod = jni::CallLongMethod,
        .CallLongMethodV = jni::CallLongMethodV,
        .CallLongMethodA = jni::CallLongMethodA,

        .CallFloatMethod = jni::CallFloatMethod,
        .CallFloatMethodV = jni::CallFloatMethodV,
        .CallFloatMethodA = jni::CallFloatMethodA,

        .CallDoubleMethod = jni::CallDoubleMethod,
        .CallDoubleMethodV = jni::CallDoubleMethodV,
        .CallDoubleMethodA = jni::CallDoubleMethodA,

        .CallVoidMethod = jni::CallVoidMethod,
        .CallVoidMethodV = jni::CallVoidMethodV,
        .CallVoidMethodA = jni::CallVoidMethodA,

        .CallNonvirtualObjectMethod = jni::CallNonvirtualObjectMethod,
        .CallNonvirtualObjectMethodV = jni::CallNonvirtualObjectMethodV,
        .CallNonvirtualObjectMethodA = jni::CallNonvirtualObjectMethodA,

        .CallNonvirtualBooleanMethod = jni::CallNonvirtualBooleanMethod,
        .CallNonvirtualBooleanMethodV = jni::CallNonvirtualBooleanMethodV,
        .CallNonvirtualBooleanMethodA = jni::CallNonvirtualBooleanMethodA,

        .CallNonvirtualByteMethod = jni::CallNonvirtualByteMethod,
        .CallNonvirtualByteMethodV = jni::CallNonvirtualByteMethodV,
        .CallNonvirtualByteMethodA = jni::CallNonvirtualByteMethodA,

        .CallNonvirtualCharMethod = jni::CallNonvirtualCharMethod,
        .CallNonvirtualCharMethodV = jni::CallNonvirtualCharMethodV,
        .CallNonvirtualCharMethodA = jni::CallNonvirtualCharMethodA,

        .CallNonvirtualShortMethod = jni::CallNonvirtualShortMethod,
        .CallNonvirtualShortMethodV = jni::CallNonvirtualShortMethodV,
        .CallNonvirtualShortMethodA = jni::CallNonvirtualShortMethodA,

        .CallNonvirtualIntMethod = jni::CallNonvirtualIntMethod,
        .CallNonvirtualIntMethodV = jni::CallNonvirtualIntMethodV,
        .CallNonvirtualIntMethodA = jni::CallNonvirtualIntMethodA,

        .CallNonvirtualLongMethod = jni::CallNonvirtualLongMethod,
        .CallNonvirtualLongMethodV = jni::CallNonvirtualLongMethodV,
        .CallNonvirtualLongMethodA = jni::CallNonvirtualLongMethodA,

        .CallNonvirtualFloatMethod = jni::CallNonvirtualFloatMethod,
        .CallNonvirtualFloatMethodV = jni::CallNonvirtualFloatMethodV,
        .CallNonvirtualFloatMethodA = jni::CallNonvirtualFloatMethodA,

        .CallNonvirtualDoubleMethod = jni::CallNonvirtualDoubleMethod,
        .CallNonvirtualDoubleMethodV = jni::CallNonvirtualDoubleMethodV,
        .CallNonvirtualDoubleMethodA = jni::CallNonvirtualDoubleMethodA,

        .CallNonvirtualVoidMethod = jni::CallNonvirtualVoidMethod,
        .CallNonvirtualVoidMethodV = jni::CallNonvirtualVoidMethodV,
        .CallNonvirtualVoidMethodA = jni::CallNonvirtualVoidMethodA,


        .GetFieldID = jni::GetFieldID,
        .GetObjectField = jni::GetObjectField,
        .GetBooleanField = jni::GetBooleanField,
        .GetByteField = jni::GetByteField,
        .GetCharField = jni::GetCharField,
        .GetShortField = jni::GetShortField,
        .GetIntField = jni::GetIntField,
        .GetLongField = jni::GetLongField,
        .GetFloatField = jni::GetFloatField,
        .GetDoubleField = jni::GetDoubleField,

        .SetObjectField = jni::SetObjectField,
        .SetBooleanField = jni::SetBooleanField,
        .SetByteField = jni::SetByteField,
        .SetCharField = jni::SetCharField,
        .SetShortField = jni::SetShortField,
        .SetIntField = jni::SetIntField,
        .SetLongField = jni::SetLongField,
        .SetFloatField = jni::SetFloatField,
        .SetDoubleField = jni::SetDoubleField,

        .GetStaticMethodID = jni::GetStaticMethodID,

        .CallStaticObjectMethod = jni::CallStaticObjectMethod,
        .CallStaticObjectMethodV = jni::CallStaticObjectMethodV,
        .CallStaticObjectMethodA = jni::CallStaticObjectMethodA,

        .CallStaticBooleanMethod = jni::CallStaticBooleanMethod,
        .CallStaticBooleanMethodV = jni::CallStaticBooleanMethodV,
        .CallStaticBooleanMethodA = jni::CallStaticBooleanMethodA,

        .CallStaticByteMethod = jni::CallStaticByteMethod,
        .CallStaticByteMethodV = jni::CallStaticByteMethodV,
        .CallStaticByteMethodA = jni::CallStaticByteMethodA,

        .CallStaticCharMethod = jni::CallStaticCharMethod,
        .CallStaticCharMethodV = jni::CallStaticCharMethodV,
        .CallStaticCharMethodA = jni::CallStaticCharMethodA,

        .CallStaticShortMethod = jni::CallStaticShortMethod,
        .CallStaticShortMethodV = jni::CallStaticShortMethodV,
        .CallStaticShortMethodA = jni::CallStaticShortMethodA,


        .CallStaticIntMethod = jni::CallStaticIntMethod,
        .CallStaticIntMethodV = jni::CallStaticIntMethodV,
        .CallStaticIntMethodA = jni::CallStaticIntMethodA,

        .CallStaticLongMethod = jni::CallStaticLongMethod,
        .CallStaticLongMethodV = jni::CallStaticLongMethodV,
        .CallStaticLongMethodA = jni::CallStaticLongMethodA,

        .CallStaticFloatMethod = jni::CallStaticFloatMethod,
        .CallStaticFloatMethodV = jni::CallStaticFloatMethodV,
        .CallStaticFloatMethodA = jni::CallStaticFloatMethodA,

        .CallStaticDoubleMethod = jni::CallStaticDoubleMethod,
        .CallStaticDoubleMethodV = jni::CallStaticDoubleMethodV,
        .CallStaticDoubleMethodA = jni::CallStaticDoubleMethodA,

        .CallStaticVoidMethod = jni::CallStaticVoidMethod,
        .CallStaticVoidMethodV = jni::CallStaticVoidMethodV,
        .CallStaticVoidMethodA = jni::CallStaticVoidMethodA,

        .GetStaticFieldID = jni::GetStaticFieldID,
        .GetStaticObjectField = jni::GetStaticObjectField,
        .GetStaticBooleanField = jni::GetStaticBooleanField,
        .GetStaticByteField = jni::GetStaticByteField,
        .GetStaticCharField = jni::GetStaticCharField,
        .GetStaticShortField = jni::GetStaticShortField,
        .GetStaticIntField = jni::GetStaticIntField,
        .GetStaticLongField = jni::GetStaticLongField,
        .GetStaticFloatField = jni::GetStaticFloatField,
        .GetStaticDoubleField = jni::GetStaticDoubleField,

        .SetStaticObjectField = jni::SetStaticObjectField,
        .SetStaticBooleanField = jni::SetStaticBooleanField,
        .SetStaticByteField = jni::SetStaticByteField,
        .SetStaticCharField = jni::SetStaticCharField,
        .SetStaticShortField = jni::SetStaticShortField,
        .SetStaticIntField = jni::SetStaticIntField,
        .SetStaticLongField = jni::SetStaticLongField,
        .SetStaticFloatField = jni::SetStaticFloatField,
        .SetStaticDoubleField = jni::SetStaticDoubleField,

        .NewString = jni::NewString,
        .GetStringLength = jni::GetStringLength,
        .GetStringChars = jni::GetStringChars,
        .ReleaseStringChars = jni::ReleaseStringChars,

        .NewStringUTF = jni::NewStringUTF,
        .GetStringUTFLength = jni::GetStringUTFLength,
        .GetStringUTFChars = jni::GetStringUTFChars,
        .ReleaseStringUTFChars = jni::ReleaseStringUTFChars,

        .GetArrayLength = jni::GetArrayLength,

        .NewObjectArray = jni::NewObjectArray,
        .GetObjectArrayElement = jni::GetObjectArrayElement,
        .SetObjectArrayElement = jni::SetObjectArrayElement,

        .NewBooleanArray = jni::NewBooleanArray,
        .NewByteArray = jni::NewByteArray,
        .NewCharArray = jni::NewCharArray,
        .NewShortArray = jni::NewShortArray,
        .NewIntArray = jni::NewIntArray,
        .NewLongArray = jni::NewLongArray,
        .NewFloatArray = jni::NewFloatArray,
        .NewDoubleArray = jni::NewDoubleArray,

        .GetBooleanArrayElements = jni::GetBooleanArrayElements,
        .GetByteArrayElements = jni::GetByteArrayElements,
        .GetCharArrayElements = jni::GetCharArrayElements,
        .GetShortArrayElements = jni::GetShortArrayElements,
        .GetIntArrayElements = jni::GetIntArrayElements,
        .GetLongArrayElements = jni::GetLongArrayElements,
        .GetFloatArrayElements = jni::GetFloatArrayElements,
        .GetDoubleArrayElements = jni::GetDoubleArrayElements,

        .ReleaseBooleanArrayElements = jni::ReleaseBooleanArrayElements,
        .ReleaseByteArrayElements = jni::ReleaseByteArrayElements,
        .ReleaseCharArrayElements = jni::ReleaseCharArrayElements,
        .ReleaseShortArrayElements = jni::ReleaseShortArrayElements,
        .ReleaseIntArrayElements = jni::ReleaseIntArrayElements,
        .ReleaseLongArrayElements = jni::ReleaseLongArrayElements,
        .ReleaseFloatArrayElements = jni::ReleaseFloatArrayElements,
        .ReleaseDoubleArrayElements = jni::ReleaseDoubleArrayElements,

        .GetBooleanArrayRegion = jni::GetBooleanArrayRegion,
        .GetByteArrayRegion = jni::GetByteArrayRegion,
        .GetCharArrayRegion = jni::GetCharArrayRegion,
        .GetShortArrayRegion = jni::GetShortArrayRegion,
        .GetIntArrayRegion = jni::GetIntArrayRegion,
        .GetLongArrayRegion = jni::GetLongArrayRegion,
        .GetFloatArrayRegion = jni::GetFloatArrayRegion,
        .GetDoubleArrayRegion = jni::GetDoubleArrayRegion,

        .SetBooleanArrayRegion = jni::SetBooleanArrayRegion,
        .SetByteArrayRegion = jni::SetByteArrayRegion,
        .SetCharArrayRegion = jni::SetCharArrayRegion,
        .SetShortArrayRegion = jni::SetShortArrayRegion,
        .SetIntArrayRegion = jni::SetIntArrayRegion,
        .SetLongArrayRegion = jni::SetLongArrayRegion,
        .SetFloatArrayRegion = jni::SetFloatArrayRegion,
        .SetDoubleArrayRegion = jni::SetDoubleArrayRegion,

        .RegisterNatives = jni::RegisterNatives,
        .UnregisterNatives = jni::UnregisterNatives,

        .MonitorEnter = jni::MonitorEnter,
        .MonitorExit = jni::MonitorExit,

        .GetJavaVM = jni::GetJavaVM,

        .GetStringRegion = jni::GetStringRegion,
        .GetStringUTFRegion = jni::GetStringUTFRegion,

        .GetPrimitiveArrayCritical = jni::GetPrimitiveArrayCritical,
        .ReleasePrimitiveArrayCritical = jni::ReleasePrimitiveArrayCritical,

        .GetStringCritical = jni::GetStringCritical,
        .ReleaseStringCritical = jni::ReleaseStringCritical,

        .NewWeakGlobalRef = jni::NewWeakGlobalRef,
        .DeleteWeakGlobalRef = jni::DeleteWeakGlobalRef,

        .ExceptionCheck = jni::ExceptionCheck,

        .NewDirectByteBuffer = jni::NewDirectByteBuffer,
        .GetDirectBufferAddress = jni::GetDirectBufferAddress,
        .GetDirectBufferCapacity = jni::GetDirectBufferCapacity,


        /* New JNI 1.6 Features */
        .GetObjectRefType = jni::GetObjectRefType,
};

JNIEnv s_jni_env = {
        .functions = &s_jni_native_interface,
};

JNIEnv *jni::jni_env = &s_jni_env;