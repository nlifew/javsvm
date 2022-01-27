

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

private:
    /**
     * 表示 static 块，即 <cinit> 函数的执行状态。目前分为 3 种，尚未初始化，正在初始化，初始化完成，初始化失败
     *
     * 根据虚拟机规范，在执行以下字节码前，必须先检查类的状态，保证 <cinit> 已经 *正确* 执行过
     * new, getstatic, putstatic, invokestatic
     * 调用者不应该直接访问 cinit 枚举类或 cinit 字段，而是通过检查 invoke_cinit() 的返回值判断是否成功。
     * 一旦失败，虚拟机应该尽快走异常逻辑。
     */
    enum cinit {
        NOT_INITED = 0,
        DOING_INIT = 1,
        INIT_DONE = 2,
        INIT_FAILED = -1,
    };

    volatile enum cinit cinit = NOT_INITED;

    int do_invoke_cinit() noexcept;

public:
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
     bool is_assign_from(jclass *sub) noexcept;

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
     * 检查 <cinit> 函数的执行状态。如果没有执行过，则开始向父类递归地执行;
     * 已经执行过的话不会有任何影响
     * @return 已经执行过返回一个正数，没有执行过且执行完没有异常，返回 0;
     * 失败返回 -1
     */
     [[nodiscard]]
     int invoke_cinit() noexcept;
};
} // namespace javsvm

#endif

