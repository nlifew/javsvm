

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
     */
    static jclass *load_class(const char *name);


    /**
     * 获取 java 层的 Class 对象在本地的指针
     * 如果失败返回 nullptr
     */
     static jclass *of(jref obj) noexcept;

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

    /**
     * 判断后面的对象是否是当前 class 的子类
     */
    bool is_instance(jref obj) noexcept;

    /**
     * 判断后面的类是否是当前 class 的子类
     */
     bool is_assign_from(jclass *sub) const noexcept;

    // todo: 分配内存，创建对象，但不执行构造函数
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

