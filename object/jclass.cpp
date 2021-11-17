

#include "jclass.h"
#include "jfield.h"
#include "jmethod.h"
//#include <vm/jvm.h>

//#include <class/jclass_file.h>
//#include <engine/engine.h>
//#include <memory>


// #include "classloader/bootstrap_loader.h"

using namespace javsvm;


jfield* jclass::get_field(const char *_name, const char *_sig) const noexcept
{
    jfield f;
    f.name = name;
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
    jfield *f = get_field(_name, _sig);
    if (f != nullptr && (f->access_flag & jclass_field::ACC_STATIC) != 0) {
        return f;
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

//jclass* jclass::find_class(const char *name)
//{
//    return jvm::get().class_loader().load_class(name);
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