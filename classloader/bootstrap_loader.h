
#ifndef JAVSVM_BOOTSTRAP_LOADER_H
#define JAVSVM_BOOTSTRAP_LOADER_H


#include "../utils/global.h"
#include "../concurrent/recursive_lock.h"

// #include <mutex>
#include <string>
#include <unordered_map>

namespace javsvm
{

class jobject;
class jclass;
class jclass_file;
class jmethod_area;
class jheap;
class jvm;

class bootstrap_loader
{
private:
    /**
     * 类缓存池
     */ 
    std::unordered_map<std::string, jclass*> m_classes;

    /**
     * 使用可重入的读写锁
     */
    recursive_lock m_lock;

    /**
     * 标志位，表示 java.lang.Object 类是否已经初始化(创建出对应的 java.lang.Class 对象)
     */
    bool m_has_full_object_class = false;


    void *m_uninitialized_class_queue = nullptr;

    /**
     * 内存分配器，jmethod, jfield 等对象都在这里分配
     */
    jmethod_area &m_allocator;

    /**
     * 寻找 class 文件，并转化为 jclass_file 格式
     */  
    static jclass_file *find_class(const char *name);

    /**
     * 保证 .class 文件内容是合法的，可靠的
     */ 
    // jclass *verify_class(jclass_file *);

    /**
     * 分配类变量，并给其赋初值
     */
    jclass *prepare_class(jclass_file *cls);

    friend class jarray;
    void gen_parent_tree(jclass *klass);

    /**
     * 创建函数表
     */ 
    void gen_method_table(jclass *klass, jclass_file *cls);

    /**
     * 拷贝父类的虚函数表
     */ 
    void copy_super_vtable(jclass *klass);

    /**
     * 创建字段表
     */
    void gen_field_table(jclass *klass, jclass_file *pFile);

    /**
     * 摆放所有的静态字段到内存中
     */ 
    void layout_static_fields(jclass *klass);


    /**
     * 摆放所有的实例字段到内存中
     */ 
    static void layout_direct_fields(jclass *klass);

    /**
     * 创建出和 klass 对应的 java 层对象 (java.lang.Class 类的实例)
     */
    jref new_class_object(jclass *klass);

    /**
     * 加载数组类型。jvm 要求数组的类加载器和其包裹类型必须是同一个类加载器，
     * 因此数组类型也必须交给 classloader 处理(隔离)，而不能交给全局的 jarray。
     */
     jclass *load_array_type(const std::string &s);

     jclass *create_primitive_type(const char *name);

public:
    explicit bootstrap_loader(jvm& mem) noexcept;

    bootstrap_loader(const bootstrap_loader &) = delete;
    bootstrap_loader &operator=(const bootstrap_loader &) = delete;
    ~bootstrap_loader() noexcept = default;

    /**
     * 根据类的全限定名找到并解析为内存中的 jclass 格式
     * 如果缓存池中已经有这个类，直接返回; 否则从磁盘中加载
     */
    jclass *load_class(const char *name);
};

} // namespace javsvm
#endif
