

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

static void calculate_slot_num(jmethod *method, jmethod_area &allocator)
{
    int& args_slot = method->args_slot;
    const char* &sig = method->sig;
    auto &return_type = method->return_type;

    args_slot = 0;

    std::vector<char> ref_table;
    ref_table.reserve(32);

    // 非静态函数的第一个参数是 this，要占一个变量槽
    if ((method->access_flag & jclass_method::ACC_STATIC) == 0) {
        args_slot += 1;
        ref_table.push_back(1);
    }

    for (int i = 1; sig[i] != ')'; i ++) {
        switch (sig[i]) {
            case 'Z':       /* boolean */
            case 'B':       /* byte */
            case 'C':       /* char */
            case 'S':       /* short */
            case 'I':       /* int */
            case 'F':       /* float */
                args_slot += 1;
                ref_table.push_back(0);
                break;
            case 'D':       /* double */
            case 'J':       /* long */
                args_slot += 2;
                ref_table.push_back(0);
                ref_table.push_back(0);
                break;
            case 'L':       /* object */
                i = (int) (strchr(sig + i + 1, ';') - sig);
                args_slot += 1;
                ref_table.push_back(1);
                break;
            case '[':       /* array */
                while (sig[i] == '[') i ++;
                if (sig[i] == 'L') i = (int) (strchr(sig + i + 1, ';') - sig);
                args_slot += 1;
                ref_table.push_back(1);
                break;
            default:
                LOGE("unknown jmethod sig: '%s'\n", sig);
                exit(1);
        }
    }

    switch (strchr(sig + 1, ')')[1]) {
        case 'V':       /* void */
            return_type = jmethod::none;
            break;
        case 'Z':       /* boolean */
        case 'B':       /* byte */
        case 'C':       /* char */
        case 'S':       /* short */
        case 'I':       /* int */
        case 'F':       /* float */
            return_type = jmethod::integer32;
            break;
        case 'J':       /* long */
        case 'D':       /* double */
            return_type = jmethod::integer64;
            break;
        case 'L':       /* object */
        case '[':       /* array */
            return_type = javsvm::jmethod::reference;
            break;
        default:
            LOGE("unknown return type: %s\n", sig);
            exit(1);
    }

    ref_table.push_back(0); // 终止字符串 ?
    if (args_slot > 0) {
        method->args_ref_table = allocator.calloc_type<char>(args_slot + 1);
        memcpy(method->args_ref_table, &ref_table[0], args_slot + 1);
    }
}


void jmethod::bind(jmethod_area &allocator, jclass *klass, jclass_file *cls, int index) noexcept
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
    calculate_slot_num(this, allocator);
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

//    // 如果被 synchronized 关键字修饰，加锁
//    bool synchronized = false;
//
//    if ((access_flag & jclass_method::ACC_SYNCHRONIZED) != 0
//            /* || strcmp(method->name, "<cinit>") == 0 */) { // [1]
//
//        // [1]. 不处理 <clinit>，jclass 自己会处理
//        synchronized = true;
//        jvm::get().heap.lock(ref)->lock();
//    }

    jvalue value;
    args.reset();

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

//    // 解除锁
//    if (synchronized) {
//        jvm::get().heap.lock(ref)->unlock();
//    }
    return value;
}



jvalue jmethod::invoke_static(jargs &args)
{
    if (clazz->invoke_clinit() < 0) {
        return { 0 };
    }
    return lock_and_run(this, clazz->object.get(), args);
}


jvalue jmethod::invoke_special(jref ref, jargs &args)
{
    auto object = jheap::cast(ref);

    // 检查是否是空指针对象
    if (UNLIKELY(object == nullptr)) {
        LOGE("java.lang.NullPointerException\n");
        exit(1);
    }

    return lock_and_run(this, ref, args);
}


jvalue jmethod::invoke_virtual(jref ref, jargs &args) const
{
    auto object = jheap::cast(ref);

    // 检查是否是空指针对象
    if (UNLIKELY(object == nullptr)) {
        LOGE("java.lang.NullPointerException\n");
        exit(1);
    }

    // 获取函数的真正实现
    assert(index_in_table >= 0 && index_in_table < object->klass->vtable_size);
    auto _this = object->klass->vtable[index_in_table];
    assert(jmethod::compare_to(this, _this) == 0);

    return lock_and_run(_this, ref, args);
}


jvalue jmethod::invoke_interface(jref ref, jargs &args)
{
    // 锁住堆内存，获取真实的 jobject 指针
    auto object = jheap::cast(ref);

    // 检查是否是空指针对象
    if (UNLIKELY(object == nullptr)) {
        LOGE("java.lang.NullPointerException\n");
        exit(1);
    }

    // 获取函数的真正实现

    jmethod **itable = object->klass->itable;
    const int itable_size = object->klass->itable_size;

    // 先判断缓存是否仍然有效
    int index = - index_in_table - 1;
    assert(index > -1);

    if (/* index > -1 && */ index < itable_size && jmethod::compare_to(this, itable[index]) == 0) {
        // 命中缓存，直接返回
        return lock_and_run(itable[index], ref, args);
    }

    // 缓存失效，查找接口函数表
    jmethod m, *p_method = &m;
    m.name = name;
    m.sig = sig;

    auto result = (jmethod **) bsearch(&p_method, itable, itable_size,
                           sizeof(jmethod*),
                           [](const void *_p1, const void *_p2) -> int {
                               auto p1 = (jmethod **)_p1;
                               auto p2 = (jmethod **)_p2;
                               return jmethod::compare_to(*p1, *p2);
                           });
    assert(result != nullptr);

    index = (int) (result - itable);
    index_in_table = - index - 1;

    assert(jmethod::compare_to(itable[index], this) == 0);
    return lock_and_run(itable[index], ref, args);
}
