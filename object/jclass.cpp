

#include "jclass.h"
#include "jfield.h"
#include "jmethod.h"
#include "../engine/engine.h"

//#include <class/jclass_file.h>
//#include <engine/engine.h>
//#include <memory>


// #include "classloader/bootstrap_loader.h"

using namespace javsvm;




jfield* jclass::get_field(const char *_name, const char *_sig) const noexcept
{
    jfield f;
    f.name = _name;
    f.sig = _sig;

    for (const jclass *klass = this; klass; klass = klass->super_class) {
        using cmp_t = int (*)(const void *, const void *);

        void *_result = bsearch(&f, klass->field_tables,
                                klass->field_table_size,
                                sizeof(jfield),
                                (cmp_t)jfield::compare_to);
        if (_result != nullptr) {
            return (jfield *)_result;
        }
    }
    return nullptr;
}


jfield *jclass::get_static_field(const char *_name, const char *_sig) const noexcept
{
    jfield f;
    f.name = _name;
    f.sig = _sig;

    for (const jclass *klass = this; klass; klass = klass->super_class) {
        using cmp_t = int (*)(const void *, const void *);

        auto _result = (jfield *) bsearch(&f, klass->field_tables,
                                klass->field_table_size,
                                sizeof(jfield),
                                (cmp_t)jfield::compare_to);
        if (_result != nullptr && (_result->access_flag & jclass_field::ACC_STATIC) != 0) {
            return _result;
        }
        // 查询接口类
        for (int j = 0, z = klass->interface_num; j < z; j ++) {
            auto interface = klass->interfaces[j];
            _result = (jfield *) bsearch(&f, interface->field_tables,
                                         interface->field_table_size,
                                         sizeof(jfield),
                                         (cmp_t)jfield::compare_to);
            if (_result != nullptr && (_result->access_flag & jclass_field::ACC_STATIC) != 0) {
                return _result;
            }
        }
    }
    return nullptr;
}


jmethod* jclass::get_method(const char *_name, const char *_sig) const noexcept
{
    jmethod m;
    m.name = _name;
    m.sig = _sig;

    for (const jclass *klass = this; klass; klass = klass->super_class) {
        using cmp_t = int (*)(const void *, const void *);

        void *result = bsearch(&m, klass->method_tables,
                               klass->method_table_size,
                               sizeof(jmethod),
                               (cmp_t)jmethod::compare_to);
        if (result != nullptr) {
            return (jmethod *)result;
        }
    }

    return nullptr;
}

jmethod* jclass::get_virtual_method(const char *_name, const char *_sig) const noexcept
{
    jmethod m, *p_method = &m;
    m.name = _name;
    m.sig = _sig;

    void *result = bsearch(&p_method, vtable, vtable_size, sizeof(jmethod*),
                           [](const void *_p1, const void *_p2) -> int {
        auto p1 = (jmethod **)_p1;
        auto p2 = (jmethod **)_p2;
        return jmethod::compare_to(*p1, *p2);
    });
    return result == nullptr ? nullptr : *(jmethod **)result;
}


jmethod *jclass::get_static_method(const char *_name, const char *_sig) const noexcept
{
    jmethod *m = get_method(_name, _sig);
    if (m != nullptr && (m->access_flag & jclass_method::ACC_STATIC) != 0) {
        return m;
    }
    return nullptr;
}


bool jclass::is_instance(jref ref) noexcept
{
    auto ptr = jvm::get().heap.lock(ref);
    if (ptr == nullptr) {
        return false;
    }
    const auto *s = ptr->klass;
    const auto *t = this;
    if (s == t || s->cached_parent == t) {
        return true;
    }

    // 脱去数组层
    while (s->component_type && t->component_type) {
        s = s->component_type;
        t = t->component_type;
    }
    // 没有任何类继承自数组类型
    if (t->component_type) {
        return false;
    }
    // 遍历 s 的继承树，线性查找 t
    for (int i = 0, z = s->parent_tree_size; i < z; i ++) {
        if (s->parent_tree[i] == t) {
            ptr->klass->cached_parent = this;
            return true;
        }
    }
    return false;
}

bool jclass::is_assign_from(jclass *sub) noexcept
{
    if (sub == nullptr) {
        return false;
    }
    const auto pt = sub->parent_tree;
    for (int i = 0, z = sub->parent_tree_size; i < z; i ++) {
        if (pt[i] == this) {
            sub->cached_parent = this;
            return true;
        }
    }
    return false;
}


jclass *jclass::of(jref obj) noexcept
{
    auto &vm = jvm::get();
    jclass *java_lang_Class = vm.bootstrap_loader.load_class("java/lang/Class");
    if (java_lang_Class == nullptr) {
        LOGE("javsvm::of: failed to load java/lang/Class\n");
        exit(1);
    }
    if (! java_lang_Class->is_instance(obj)) {
        return nullptr;
    }

    jfield *mNativePtr = java_lang_Class->get_field("mNativePtr", "Ljava/lang/Class;");
    if (mNativePtr == nullptr) {
        LOGE("javsvm::of: failed to find field: mNativePtr\n");
        exit(1);
    }
    return (jclass*) mNativePtr->get(obj).j;
}




jclass *jclass::load_class(const char *name)
{
    auto &vm = jvm::get();
    auto &stack = vm.env().stack;
    auto stack_frame = stack.top();

    // 栈帧为空，说明还没有走到 java 函数，使用初始类加载器
    if (stack_frame == nullptr) {
        return vm.bootstrap_loader.load_class(name);
    }
    // 找到栈顶的类的加载器
    auto loader = stack_frame->method->clazz->loader;
    auto loader_ptr = vm.heap.lock(loader);

    // 如果栈顶函数的类加载器为 null，使用系统类加载器
    if (loader_ptr == nullptr) {
        return vm.bootstrap_loader.load_class(name);
    }
    static jmethod *java_lang_ClassLoader_loadClass = nullptr;
    if (java_lang_ClassLoader_loadClass == nullptr) {
        java_lang_ClassLoader_loadClass = loader_ptr->klass->get_virtual_method("findClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    }
    loader_ptr.reset(); // 不用的指针及时释放

    // 走到这里需要调用 java 层的 Class<?> ClassLoader.loadClass(String) 函数
    slot_t buff[2];
    memset(buff, 0, sizeof(buff));

    buff[0] = (slot_t) loader;
    buff[1] = (slot_t) vm.string.find_or_new(name);

    jargs args(buff);
    jvalue val = java_lang_ClassLoader_loadClass->invoke_virtual(loader, args);

    auto class_ptr = vm.heap.lock(val.l);
    if (class_ptr == nullptr) {
        // 加载失败了，直接返回
        return nullptr;
    }

    // 获取这个 Class 对象在 native 层的指针，强转后返回
    static jfield *java_lang_Class_mNativePtr = nullptr;
    if (java_lang_Class_mNativePtr == nullptr) {
        java_lang_Class_mNativePtr = class_ptr->klass->get_field("mNativePtr", "J");
    }
    val = java_lang_Class_mNativePtr->get(val.l);
    return (jclass *) val.j;
}


jref jclass::new_instance() noexcept
{
    if (invoke_cinit() < 0) {
        return nullptr;
    }

    auto &heap = jvm::get().heap;
    jref ref = heap.malloc_bytes(object_size);

    auto ptr = heap.lock(ref);
    ptr->klass = const_cast<jclass*>(this);
    return ref;
}


jref jclass::new_instance(jmethod *m, ...) noexcept
{
    va_list ap;
    va_start(ap, m);
    auto ret = new_instance(m, ap);
    va_end(ap);
    return ret;
}

/**
 * va_list 转 slot 数组
 * 调用者需要 delete[] 释放内存
 */
static javsvm::slot_t *to_args(jmethod *method, jref obj, va_list ap)
{
    auto args = new javsvm::slot_t[method->args_slot];
    memset(args, 0, sizeof(slot_t) * method->args_slot);

    javsvm::jargs _args(args);

//    if ((method->access_flag & jclass_method::ACC_STATIC) == 0) {
        _args.next<jref>() = obj;
//    }

    const char *sig = method->sig;
    for (int i = 1; sig[i] != ')'; i ++) {
        switch (sig[i]) {
            case 'Z':       /* boolean */
                _args.next<jboolean>() = va_arg(ap, jint) != 0;
                break;
            case 'B':       /* byte */
                _args.next<jbyte>() = va_arg(ap, jint) & 0xFF;
                break;
            case 'C':       /* char */
                _args.next<jchar>() = va_arg(ap, jint) & 0xFFFF;
                break;
            case 'S':       /* short */
                _args.next<jshort>() = va_arg(ap, jint) & 0xFFFF;
                break;
            case 'I':       /* int */
                _args.next<jint>() = va_arg(ap, jint);
                break;
            case 'J':       /* long */
                _args.next<jlong>() = va_arg(ap, jlong);
                break;
            case 'F':       /* float */
                _args.next<jfloat>() = va_arg(ap, jdouble);
                break;
            case 'D':       /* double */
                _args.next<jdouble>() = va_arg(ap, jdouble);
                break;
            case 'L':       /* object */
                _args.next<javsvm::jref>() = va_arg(ap, jref);
                i = (int) (strchr(sig + i + 1, ';') - sig);
                break;
            case '[':       /* array */
                _args.next<javsvm::jref>() = va_arg(ap, jref);
                while (sig[i] == '[') i ++;
                if (sig[i] == 'L') i = (int) (strchr(sig + i + 1, ';') - sig);
                break;
            default:
                LOGE("to_args: unknown jmethod sig: '%s'\n", sig);
                break;
        }
    }
    return args;
}

jref jclass::new_instance(jmethod *m, va_list ap) noexcept
{
    jref obj = new_instance();

    auto *args = to_args(m, obj, ap);
    std::unique_ptr<slot_t, void(*)(const slot_t *)> args_guard(
            args, [](const slot_t *ptr) { delete[] ptr; });

    jargs _args(args);
    m->invoke_special(obj, _args);
    return obj;
}


int jclass::invoke_cinit() noexcept
{
//    LOGD("invoke_cinit: start with %s\n", name);
    if (cinit == INIT_DONE) {
//        LOGD("invoke_cinit: <cinit> has been invoked, nothing to do\n");
        return 1;
    }
    if (cinit == INIT_FAILED) {
//        LOGW("invoke_cinit: <cinit> invoke failed\n");
        return -1;
    }


    LOGD("invoke_cinit: start with %s\n", name);

    // 接下来无非两种状态，正在初始化和尚未初始化。对于前者，常见于多线程操作:
    // 比如线程 A 在执行 <cinit> 时线程调度出去(或者执行了耗时操作)，线程 B 尝试创建此类的实例，此时
    // 我们必须阻塞线程 B，直到线程 A 执行完成。还有一种就是父类在 <cinit> 里创建子类的对象时，子类
    // 先检查父类，也会遇到正在初始化的情况。

    // 先锁住类
    // NOTE: 正常情况下类对象 object 是绝对不可能为 nullptr 的，但我们也要考虑到 object 和 class 类加载时的特殊情况
    // java.lang.Object 类在加载时会创建一个 java.lang.Class 的伴随对象，也就是调用 java.lang.Class 的构造函数。
    // 后者又会调 Object 的 cinit，但 Object 类此时还没有完成初始化，因此锁住类时一定会出错
    jobject_ptr ptr = jvm::get().heap.lock(object);


    LOGD("invoke_cinit: lock on the Class object\n");
    if (ptr != nullptr) {
        auto ok = ptr->lock();
        assert(ok == 0);
    }

    std::unique_ptr<jclass, void (*) (const jclass *)> lock_guard(this, [](const jclass *clz) {
        auto ptr = jvm::get().heap.lock(clz->object);
        if (ptr != nullptr) {
            auto ok = ptr->unlock();
            assert(ok == 0);
        }
    });

    // double check
    switch (cinit) {
        case DOING_INIT: {
            // 这种情况也就是上面所说的父类调子类。放行
            LOGD("invoke_cinit: <cinit> is invoking, just take a chance\n");
            return 1;
        }
        case INIT_DONE: {
            LOGD("invoke_cinit: <cinit> has been invoked, nothing to do\n");
            return 1;
        }
        case INIT_FAILED: {
            LOGW("invoke_cinit: <cinit> invoke failed\n");
            return -1;
        }
        case NOT_INITED: // nothing to do
            break;
    }

    // 不用的指针及时释放
    ptr.reset();
    return do_invoke_cinit();
}


static inline int get_constant_value(jclass_attr_constant *attr, jclass_const_pool &pool, jvalue *ret) noexcept
{
    jclass_const *const_value = pool.child_at(attr->constant_value_index - 1);

    switch (const_value->tag) {
        case jclass_const_int::TAG: {
            u4 i = ((jclass_const_int *)const_value)->bytes;
            ret->i = *(int *)&i;
            return 0;
        }
        case jclass_const_float::TAG: {
            u4 f = ((jclass_const_float *)const_value)->bytes;
            ret->f = *(jfloat *)&f;
            return 0;
        }
        case jclass_const_long::TAG: {
            u8 l = ((::jclass_const_long *)const_value)->bytes;
            ret->j = *(jlong *)&l;
            return 0;
        }
        case jclass_const_double::TAG: {
            u8 d = ((jclass_const_double *)const_value)->bytes;
            ret->d = *(jdouble *)&d;
            return 0;
        }
        case jclass_const_string::TAG: {
            // 这里比较复杂，因为要创建出一个新的 java.lang.String 对象
            auto *s = ((jclass_const_string *) const_value);
            auto *utf8 = pool.cast<jclass_const_utf8>(s->index);
            ret->l = jvm::get().string.find_or_new((char *)utf8->bytes);
            return 0;
        }
        default:
            LOGD("get_constant_value: ignore unknown const_value_tag %d\n", const_value->tag);
            return -1;
    }
}


int jclass::do_invoke_cinit() noexcept
{
    cinit = DOING_INIT;

    // 递归调用父类的
    for (jclass *i = super_class; i; i = i->super_class) {
        LOGD("invoke_cinit: super class %s\n", i->name);
        if (i->invoke_cinit() < 0) {
            LOGE("invoke_cinit: something bad occurred\n");
            cinit = INIT_FAILED;
            return -1;
        }
    }
    // 接口类
    for (int i = 0; i < interface_num; i ++) {
        auto &interface = interfaces[i];
        LOGD("invoke_cinit: interface class %s\n", interface->name);
        if (interface->invoke_cinit() < 0) {
            LOGE("invoke_cinit: something bad occurred\n");
            cinit = INIT_FAILED;
            return -1;
        }
    }

    // 处理常量字段
    LOGD("invoke_cinit: layout constant fields: %d\n", field_table_size);
    jclass_const_pool &pool = class_file->constant_pool;
    for (int i = 0; i < field_table_size; i++) {
        jfield *field = field_tables + i;
        jclass_field *src = field->orig;

        LOGD("invoke_cinit: layout constant fields: %s\n", field->name);
        jvalue val;

        for (int j = 0, z = src->attributes_count; j < z; j++) {
            auto attr = src->attributes[j]->cast<jclass_attr_constant>();
            if (attr == nullptr) {
                continue;
            }
            if (get_constant_value(attr, pool, &val) == 0) {
                field->set(nullptr, val);
            }
        }
    }

    // 创建常量有可能失败，检查下
    if (check_exception() != nullptr) {
        LOGE("invoke_cinit: failed to create constant value\n");
        cinit = javsvm::jclass::INIT_FAILED;
        return -1;
    }


    jmethod *m = get_static_method("<clinit>", "()V");
    if (m != nullptr) {
        LOGI("invoke_cinit: <cinit> found, invoke\n");
        jargs args(nullptr);
        m->invoke_static(args);

        if (check_exception() != nullptr) {
            // 坏了，有异常发生
            LOGE("invoke_cinit: something bad occurred\n");
            cinit = javsvm::jclass::INIT_FAILED;
            return -1;
        }
    }
    else {
        LOGD("invoke_cinit: <cinit> NOT found, ignore\n");
    }


    LOGD("invoke_cinit: congratulations, class '%s' done\n", name);
    cinit = javsvm::jclass::INIT_DONE;
    return 0;
}