

#ifndef JAVSVM_JCLASS_H
#define JAVSVM_JCLASS_H

#include "../utils/global.h"
#include "../vm/jvm.h"
#include "jobject.h"
namespace javsvm
{

struct jclass_file;
struct jfield;
struct jmethod;

struct jclass
{
    /**
     * 使用调用者的类加载器加载新的类
     * todo
     */
    static jclass *find_class(const char *name) { return jvm::get().bootstrap_loader.load_class(name); }

    const char *name = nullptr;
    u4 access_flag = 0;
    jclass_file *class_file = nullptr;

    jref object = nullptr;
    jref loader = nullptr;

    jclass *component_type = nullptr;

    jclass *super_class = nullptr;
    jclass **interfaces = nullptr;
    int interface_num = 0;

    int class_size = 0;
    int object_size = 0;

    jfield *field_tables = nullptr;
    int field_table_size = 0;

    jmethod *method_tables = nullptr;
    int method_table_size = 0;

    jmethod **vtable = nullptr;
    int vtable_size = 0;

    void *data = nullptr;


    int parent_tree_size = 0;
    jclass **parent_tree = nullptr;
    jclass *cached_parent = nullptr;


    jfield *get_field(const char *_name, const char *_sig) const noexcept;
    jfield *get_static_field(const char *_name, const char *_sig) const noexcept;

    jmethod *get_method(const char *_name, const char *_sig) const noexcept;
    jmethod *get_static_method(const char *_name, const char *_sig) const noexcept;
    jmethod *get_virtual_method(const char *_name, const char *_sig) const noexcept;

    bool is_instance(jref obj) noexcept;


    // todo
    jref new_instance()
    {
        auto &heap = jvm::get().heap;
        jref ref = heap.malloc_bytes(object_size);
        auto ptr = heap.lock(ref);
        ptr->klass = this;

        return ref;
    }

    jref new_instance(jmethod *m, ...)
    {
        auto &heap = jvm::get().heap;
        jref ref = heap.malloc_bytes(object_size);
        auto ptr = heap.lock(ref);
        ptr->klass = this;

        return ref;
    }
};
} // namespace javsvm

#endif

