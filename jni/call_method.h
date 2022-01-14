



static jmethodID (JNICALL GetMethodID)
        (JNIEnv *, jclass clazz, const char *name, const char *sig)
{
    javsvm::jclass *klass = to_class(clazz);
    if (klass == nullptr || name == nullptr || sig == nullptr) {
        return nullptr;
    }
    return (jmethodID) klass->get_method(name, sig);
}


static jmethodID (JNICALL GetStaticMethodID)
        (JNIEnv *, jclass clazz, const char *name, const char *sig)
{
    javsvm::jclass *klass = to_class(clazz);
    if (klass == nullptr || name == nullptr || sig == nullptr) {
        return nullptr;
    }
    return (jmethodID) klass->get_static_method(name, sig);
}



static javsvm::jvalue call_direct_method(JNIEnv*, jobject obj, jmethodID method, const javsvm::slot_t *args)
{
    auto _method = (javsvm::jmethod *) method;
    if (_method == nullptr) {
        return { .j = 0 };
    }

    javsvm::jref _obj = to_object(obj);

    if (! _method->clazz->is_instance(_obj)) {
        javsvm::throw_exp("java/lang/IncompatibleClassChangeError", "");
        return { .j = 0 };
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
    else if (! _method->is_virtual) {
        ret = _method->invoke_special(_obj, _args);
    }
    // 如果函数不是抽象函数，直接调虚函数
    else if (!HAS_FLAG(access_flag, javsvm::jclass_method::ACC_ABSTRACT)) {
        ret = _method->invoke_virtual(_obj, _args);
    }
    // 剩余的情况，按照接口的方式调用
    else {
        ret = _method->invoke_interface(_obj, _args);
    }

    return ret;
}

static javsvm::jvalue call_static_method(JNIEnv *env, jclass clazz, jmethodID method, const javsvm::slot_t *args)
{
    auto _method = (javsvm::jmethod *) method;
    if (_method == nullptr) {
        return { .j = 0 };
    }

//    javsvm::jclass *_clazz = to_class(clazz);
//    if (_clazz == nullptr) {
//        return { .j = 0 };
//    }
//
//    bool ok = false;
//    const auto parent_tree = _method->clazz->parent_tree;
//    for (int i = 0, z = _method->clazz->parent_tree_size; i < z; i ++) {
//        if (_clazz == parent_tree[i]) {
//            ok = true;
//            break;
//        }
//    }
//    if (! ok) {
//        javsvm::throw_exp("java/lang/IncompatibleClassChangeError", "");
//        return { .j = 0 };
//    }

    javsvm::jargs _args(args);
    javsvm::jvalue ret;

    // 兼容性检查
    const auto access_flag = _method->access_flag;
    if (HAS_FLAG(access_flag, javsvm::jclass_method::ACC_STATIC)) {
        ret = _method->invoke_static(_args);
    }
    else {
        // 就离谱，尝试转发到普通的函数
        return call_direct_method(env, (jobject) clazz, method, args);
    }

    return ret;
}


static javsvm::jvalue call_nonvirtual_method(JNIEnv *env, jobject obj, jclass clazz, jmethodID method, const javsvm::slot_t *args)
{
    auto _method = (javsvm::jmethod *) method;
    if (_method == nullptr) {
        return { .j = 0 };
    }
    // 检查一下 clazz 是否能够和 _method 所在的类匹配上
    // 如果不能，转发到普通的函数调用
    auto _clazz = (javsvm::jclass *) clazz;
    if (! _method->clazz->is_assign_from(_clazz)) {
        return call_direct_method(env, obj, method, args);
    }

    // 对上的话，重新寻找函数
    if (_method->clazz != _clazz) {
        _method = _clazz->get_method(_method->name, _method->sig);
        assert(_method != nullptr);
    }


    javsvm::jref _obj = to_object(obj);

    if (! _method->clazz->is_instance(_obj)) {
        javsvm::throw_exp("java/lang/IncompatibleClassChangeError", "");
        return { .j = 0 };
    }

    javsvm::jvalue ret;
    javsvm::jargs _args(args);

    // 兼容处理
    const auto access_flag = _method->access_flag;
    // 如果是静态函数，直接按照静态函数的方式调用
    if (HAS_FLAG(access_flag, javsvm::jclass_method::ACC_STATIC)) {
        ret = _method->invoke_static(_args);
    }
    else {
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
#undef no_return
