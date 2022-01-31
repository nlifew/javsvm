

#include "jmethod.h"
#include "jclass.h"
#include "jobject.h"
#include "../engine/engine.h"
#include "../vm/jvm.h"

using namespace javsvm;


static bool is_method_virtual(jclass_method *m, jclass_const_pool& pool)
{
    int access_flag = m->access_flag;

    // 不能是 private
    if ((access_flag & jclass_method::ACC_PRIVATE) != 0) {
        return false;
    }
    // 不能是 static 
    if ((access_flag & jclass_method::ACC_STATIC) != 0) {
        return false;
    }
    // 不能是构造函数
    auto name = pool.cast<jclass_const_utf8>(m->name_index);
    if (strcmp((char*) name->bytes, "<init>") == 0) {
        return false;
    }

    return true;
}

/**
 * 计算函数参数和返回值的变量槽数
 * NOTE: 当前的实现是不安全的，并没有对 sig 做校验
 */ 

static void calculate_slot_num(jmethod *method)
{
    int& args_slot = method->args_slot;
    int& return_slot = method->return_slot; 
    const char* &sig = method->sig;

    args_slot = 0;
    return_slot = 0;


    for (int i = 1; sig[i] != ')'; i ++) {
        switch (sig[i]) {
            case 'Z':       /* boolean */
            case 'B':       /* byte */
            case 'C':       /* char */
            case 'S':       /* short */
            case 'I':       /* int */
            case 'F':       /* float */
                args_slot += 1;
                break;
            case 'D':       /* double */
            case 'J':       /* long */
                args_slot += 2;
                break;
            case 'L':       /* object */
                i = (int) (strchr(sig + i + 1, ';') - sig);
                args_slot += 1;
                break;
            case '[':       /* array */
                while (sig[i] == '[') i ++;
                if (sig[i] == 'L') i = (int) (strchr(sig + i + 1, ';') - sig);
                args_slot += 1;
                break;
            default:
                LOGE("unknown jmethod sig: '%s'\n", sig);
        }
    }
    // 非静态函数的第一个参数是 this，要占一个变量槽
    if ((method->access_flag & jclass_method::ACC_STATIC) == 0) {
        args_slot += 1;
    }

    switch (strchr(sig + 1, ')')[1]) {
        case 'Z':       /* boolean */
        case 'B':       /* byte */
        case 'C':       /* char */
        case 'S':       /* short */
        case 'I':       /* int */
        case 'F':       /* float */
        case 'L':       /* object */
        case '[':       /* array */
            return_slot = 1;
            break;
        case 'D':       /* double */
        case 'J':       /* long */
            return_slot = 2;
            break;
    }
}


void jmethod::bind(jclass *klass, jclass_file *cls, int index)
{
    // 绑定原始 jclass_method 对象
    clazz = klass;
    orig = cls->methods + index;
    access_flag = orig->access_flag;
    name = (char *)cls->constant_pool.cast<jclass_const_utf8>(orig->name_index)->bytes;
    sig = (char *) cls->constant_pool.cast<jclass_const_utf8>(orig->descriptor_index)->bytes;

    // 判断虚函数
    is_virtual = is_method_virtual(orig, cls->constant_pool);


    // 初始化函数入口地址
    entrance.code_func = nullptr;
    entrance.jni_func = nullptr;

    for (int i = 0, n = orig->attributes_count; i < n; i++) {
        auto attr = orig->attributes[i]->cast<jclass_attr_code>();
        if (attr != nullptr) {
            entrance.code_func = attr;
            break;
        }
    }
    if (orig->access_flag & jclass_method::ACC_NATIVE) {
        entrance.jni_func = nullptr;
    }

    // 计算参数占用的局部变量槽
    calculate_slot_num(this);
}


/**
 * 通用的逻辑，包含了锁检查，入口点检查等
 * @param method 要执行的函数
 * @param ref 需要执行函数的对象。如果是静态函数，应该是这个函数所属的 class 对象
 * @param args 函数执行需要传递的参数
 */
static jvalue lock_and_run(jmethod *method, jref ref, jargs &args)
{
    auto access_flag = method->access_flag;

    // 如果被 synchronized 关键字修饰，加锁
    bool synchronized = false;

    if ((access_flag & jclass_method::ACC_SYNCHRONIZED) != 0
            /* || strcmp(method->name, "<cinit>") == 0 */) { // [1]

        // [1]. 不处理 <cinit>，jclass 自己会处理
        synchronized = true;
        jvm::get().heap.lock(ref)->lock();
    }

    jvalue value;

    // 如果被 native 关键字修饰，跳转到 jni 引擎
    if ((access_flag & jclass_method::ACC_NATIVE) != 0) {
        value = run_jni(method, ref, args);
    }
    // 普通的 java 函数
    else {
        if (method->entrance.code_func == nullptr) {
            LOGE("java.lang.AbstractMethodError\n");
            exit(1);
        }
        value = run_java(method, ref, args);
    }

    // 解除锁
    if (synchronized) {
        jvm::get().heap.lock(ref)->unlock();
    }
    return value;
}



jvalue jmethod::invoke_static(jargs &args)
{
    if (clazz->invoke_cinit() < 0) {
        return { 0 };
    }
    return lock_and_run(this, clazz->object, args);
}


jvalue jmethod::invoke_special(jref ref, jargs &args)
{
    auto object = jvm::get().heap.lock(ref);

    // 检查是否是空指针对象
    if (object == nullptr) {
        LOGE("java.lang.NullPointerException\n");
        exit(1);
    }

    // 释放 jobject 指针
    object.reset();

    return lock_and_run(this, ref, args);
}


jvalue jmethod::invoke_virtual(jref ref, jargs &args) const
{
    auto object = jvm::get().heap.lock(ref);

    // 检查是否是空指针对象
    if (object == nullptr) {
        LOGE("java.lang.NullPointerException\n");
        exit(1);
    }

    // 获取函数的真正实现
    auto _this = object->klass->vtable[index_in_table];

    // 释放 jobject 指针
    object.reset();

    return lock_and_run(_this, ref, args);
}


jvalue jmethod::invoke_interface(jref ref, jargs &args)
{
    // 锁住堆内存，获取真实的 jobject 指针
    auto object = jvm::get().heap.lock(ref);


    // 检查是否是空指针对象
    if (object == nullptr) {
        LOGE("java.lang.NullPointerException\n");
        exit(1);
    }

    // 获取函数的真正实现
    jmethod *_this = object->klass == clazz ?
            this :
            object->klass->get_virtual_method(name, sig);

    // 释放 jobject 指针
    object.reset();

    return lock_and_run(_this, ref, args);
}
