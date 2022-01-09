

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

bool jclass::is_assign_from(jclass *sub) const noexcept
{
    if (sub == nullptr) {
        return false;
    }
    const auto pt = sub->parent_tree;
    for (int i = 0, z = sub->parent_tree_size; i < z; i ++) {
        if (pt[i] == this) {
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
    auto &stack = vm.env().stack();
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
    buff[1] = (slot_t) vm.string_pool.find_or_new(name);

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

//jclass* jclass::find_class(const char *name)
//{
//    return jvm::get().bootstrap_loader().load_class(name);
//}

/*
jref jclass::new_instance()
{

}

jref jclass::new_instance(jmethod *m, ...)
{
    va_list args;
    va_start(args, m);
    args_wrapper wrapper = jmethod::make(args);
    va_end(args);

    jref ref = new_instance();
    m->invoke_special(ref, wrapper);
    return ref;
}
*/