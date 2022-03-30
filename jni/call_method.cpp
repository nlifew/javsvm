
#include "jni_utils.h"
#include "jni_env.h"

namespace jni
{

jmethodID (JNICALL GetMethodID)
        (JNIEnv *, jclass clazz, const char *name, const char *sig) {
    safety_area_guard guard;

    javsvm::jclass *klass = to_class(clazz);
    if (klass == nullptr) {
        return nullptr;
    }
    return (jmethodID) klass->get_method(name, sig);
}

jmethodID (JNICALL GetStaticMethodID)
        (JNIEnv *, jclass clazz, const char *name, const char *sig) {
    safety_area_guard guard;

    javsvm::jclass *klass = to_class(clazz);
    if (klass == nullptr) {
        return nullptr;
    }
    return (jmethodID) klass->get_static_method(name, sig);
}


static javsvm::jvalue call_direct_method(JNIEnv *, jobject obj, jmethodID method, const javsvm::slot_t *args) {
    auto _method = (javsvm::jmethod *) method;
    if (_method == nullptr) {
        return {.j = 0};
    }

    javsvm::jref _obj = to_object(obj);

    // methodId 必须来自 obj 的类或其父类，否则抛出异常
    if (!_method->clazz->is_instance(_obj)) {
        javsvm::throw_exp("java/lang/IncompatibleClassChangeError", "");
        return {.j = 0};
    }

    javsvm::jargs _args(args);
    javsvm::jvalue ret;

    // jni 允许调用构造函数，因此不再做额外的检查

    // 兼容处理
    // 如果是静态函数，直接按照静态函数的方式调用
    const auto access_flag = _method->access_flag;
    if (HAS_FLAG(access_flag, javsvm::jclass_method::ACC_STATIC)) {
        ret = _method->invoke_static(_args);
    }
        // 如果不是虚函数(构造函数和私有函数。静态函数会走前面的分支而不是这里)，按照私有函数调用
    else if (!_method->is_virtual) {
        ret = _method->invoke_special(_obj, _args);
    }
        // 如果函数不是来自接口类，直接调虚函数
    else if (!HAS_FLAG(_method->clazz->access_flag, javsvm::jclass_file::ACC_INTERFACE)) {
        ret = _method->invoke_virtual(_obj, _args);
    }
        // 剩余的情况，按照接口的方式调用
    else {
        ret = _method->invoke_interface(_obj, _args);
    }

    return ret;
}


static javsvm::jvalue call_static_method(JNIEnv *env, jclass clazz, jmethodID method, const javsvm::slot_t *args) {
    auto _method = (javsvm::jmethod *) method;
    if (_method == nullptr) {
        return {.j = 0};
    }

    javsvm::jargs _args(args);
    javsvm::jvalue ret;

    // 兼容性检查
    const auto access_flag = _method->access_flag;
    if (HAS_FLAG(access_flag, javsvm::jclass_method::ACC_STATIC)) {
        ret = _method->invoke_static(_args);
    } else {
        // 进入这个分支，说明 _method 是非静态函数，开发者错误使用了 API.
        // 我们只能猜测 clazz 是某个类的对象，他真正想调用的是 CallMethod()
        return call_direct_method(env, (jobject) clazz, method, args);
    }

    return ret;
}


static javsvm::jvalue
call_nonvirtual_method(JNIEnv *env, jobject obj, jclass, jmethodID method, const javsvm::slot_t *args) {
    auto _method = (javsvm::jmethod *) method;
    if (_method == nullptr) {
        return {.j = 0};
    }

    javsvm::jvalue ret;
    javsvm::jargs _args(args);

    // 到底调哪个函数取决于 method, 和传进来的 class 无关

    // 兼容处理
    const auto access_flag = _method->access_flag;
    if (HAS_FLAG(access_flag, javsvm::jclass_method::ACC_STATIC)) {
        ret = _method->invoke_static(_args);
    } else {
        javsvm::jref _obj = to_object(obj);

        if (!_method->clazz->is_instance(_obj)) {
            javsvm::throw_exp("java/lang/IncompatibleClassChangeError", "");
            return {.j = 0};
        }
        ret = _method->invoke_special(_obj, _args);
    }
    return ret;
}


#define return_type jobject
#define name Object

#include "call_method_gen.h"

#define return_type jboolean
#define name Boolean
#include "call_method_gen.h"


#define return_type jbyte
#define name Byte
#include "call_method_gen.h"


#define return_type jchar
#define name Char
#include "call_method_gen.h"


#define return_type jshort
#define name Short
#include "call_method_gen.h"


#define return_type jint
#define name Int
#include "call_method_gen.h"


#define return_type jlong
#define name Long
#include "call_method_gen.h"


#define return_type jfloat
#define name Float
#include "call_method_gen.h"


#define return_type jdouble
#define name Double
#include "call_method_gen.h"


#define return_type void
#define name Void
#define no_return
#include "call_method_gen.h"

}