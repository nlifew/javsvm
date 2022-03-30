

#ifndef JAVSVM_JCLASS_H
#define JAVSVM_JCLASS_H

#include "../utils/global.h"
#include "../gc/gc_root.h"

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

    gc_root object = nullptr;
    gc_root loader = nullptr;

    jclass *component_type = nullptr;

    jclass *super_class = nullptr;
    jclass **interfaces = nullptr;
    int interface_num = 0;

    int object_size = 0;

    jfield *field_tables = nullptr;
    int field_table_size = 0;

    int *ref_tables = nullptr;
    int ref_table_size = 0;

    jmethod *method_tables = nullptr;
    int method_table_size = 0;

    jmethod **vtable = nullptr;
    int vtable_size = 0;

    jmethod **itable = nullptr;
    int itable_size = 0;

    char *data = nullptr;


    int parent_tree_size = 0;
    jclass **parent_tree = nullptr;
    const jclass *cached_parent = nullptr;

//    jfield **direct_object_fields = nullptr;
//    int direct_object_field_num = 0;


    /**
     * 表示 static 块，即 <clinit> 函数的执行状态。目前分为 3 种，尚未初始化，正在初始化，初始化完成，初始化失败
     *
     * 根据虚拟机规范，在执行以下字节码前，必须先检查类的状态，保证 <clinit> 已经 *正确* 执行过
     * new, getstatic, putstatic, invokestatic
     * 调用者不应该直接访问 clinit 枚举类或 clinit 字段，而是通过检查 invoke_clinit() 的返回值判断是否成功。
     * 一旦失败，虚拟机应该尽快走异常逻辑。
     */
    enum clinit_t {
        NOT_INITED = 0,
        DOING_INIT = 1,
        INIT_DONE = 2,
        INIT_FAILED = -1,
    };

    volatile enum clinit_t clinit = NOT_INITED;
private:
    int do_invoke_clinit() noexcept;

public:
    jfield *get_field(const char *_name, const char *_sig) const noexcept;
    jfield *get_static_field(const char *_name, const char *_sig) const noexcept;

    jmethod *get_method(const char *_name, const char *_sig) const noexcept;
    jmethod *get_static_method(const char *_name, const char *_sig) const noexcept;
    jmethod *get_virtual_method(const char *_name, const char *_sig) const noexcept;
    jmethod *get_interface_method(const char *_name, const char *_sig) const noexcept;

    /**
     * 判断后面的对象是否是当前 class 的子类
     */
    bool is_instance(jref obj) const noexcept;

    /**
     * 判断后面的类是否是当前 class 的子类
     */
    bool is_assign_from(jclass *sub) const noexcept;

    /**
     * 分配内存，创建对象，但不执行构造函数
     * @return jref
     */
    [[nodiscard]]
    jref new_instance() noexcept;

    /**
     * 分配内存，并执行构造函数
     * @param m <init>
     * @param ap 构造函数的参数
     * @return jref
     */
    [[nodiscard]]
    jref new_instance(jmethod *m, va_list ap) noexcept;

    /**
     *
     * 分配内存，并执行构造函数
     * @param m <init>
     * @param ... 构造函数的参数
     * @return jref
     */
    [[nodiscard]]
    jref new_instance(jmethod *m, ...) noexcept;


    /**
     * 检查 <clinit> 函数的执行状态。如果没有执行过，则开始向父类递归地执行;
     * 已经执行过的话不会有任何影响
     * @return 已经执行过返回一个正数，没有执行过且执行完没有异常，返回 0;
     * 失败返回 -1
     */
    [[nodiscard]]
    int invoke_clinit() noexcept
    {
        if (clinit == INIT_DONE) {
            return 1;
        }
        if (clinit == INIT_FAILED) {
            return -1;
        }
        return do_invoke_clinit();
    }
};
} // namespace javsvm

#endif

