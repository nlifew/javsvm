//
// Created by edz on 2022/1/29.
//

#ifndef JAVSVM_GC_ROOT_H
#define JAVSVM_GC_ROOT_H

#include "../utils/global.h"
#include "../concurrent/concurrent_set.h"

namespace javsvm
{

struct gc_ref_base
{
protected:
    volatile jref m_ptr = nullptr;
public:
    explicit gc_ref_base(jref ref) noexcept:
        m_ptr(ref)
    {
    }

//    gc_ref_base& operator=(jref ref) noexcept { m_ptr = ref; return *this; }

    bool operator==(const gc_ref_base &o) const noexcept { return m_ptr == o.m_ptr; }

    bool operator!=(const gc_ref_base &o) const noexcept { return m_ptr != o.m_ptr; }

    bool operator==(jref ref) const noexcept { return m_ptr == ref; }

    bool operator!=(jref ref) const noexcept { return m_ptr != ref; }

    jref operator*() const noexcept { return m_ptr; }

    explicit operator bool() const noexcept { return m_ptr != nullptr; }

    [[nodiscard]]
    jref get() const noexcept { return m_ptr; }

    void reset(jref ref = nullptr) noexcept { m_ptr = ref; }

    [[nodiscard]]
    virtual bool owner() const noexcept = 0;
};

struct gc_weak: gc_ref_base
{
    using ref_set_type = concurrent_set<gc_weak*>;
    static ref_set_type ref_pool;

    explicit gc_weak(jref ref) noexcept:
            gc_ref_base(ref)
    {
        ref_pool.add(this);
    }

    ~gc_weak()
    {
        ref_pool.remove(this);
    }

    gc_weak(const gc_weak &) = delete;
    gc_weak(gc_weak&&) = delete;
    gc_weak& operator=(const gc_weak&) = delete;
    gc_weak& operator=(gc_weak&&) = delete;

    [[nodiscard]]
    bool owner() const noexcept override { return false; }
};


/**
 * 标记当前对象是一个 GcRoot 节点
 * 被 gc_root 结构体包裹的对象会直接参与到根节点枚举中来
 *
 * 下面几个为常见的 RcRoot:
 * 1. 核心系统类(来自 rt.jar 的类)，如 java.lang.*, java.util.* 类
 * 2. JNI 局部引用，包括进入 JNI 时虚拟机自动创建的，和 JNIEnv->NewLocalRef() 手动创建的
 * 3. JNI 全局引用，即使用 JNIEnv->NewGlobalRef() 创建的
 * 4. 已经开始的，尚未停止的 java.lang.Thread 对象
 * 5. 正在作为锁使用的对象
 * 6. Java 运行时栈中，局部变量表和操作数栈中保存的，和异常对象引用
 * 7. Java 中的静态变量
 * 8. Native 栈中使用的对象，包括 JNI 中的，也包括 JVM 自己使用的。比如用于文件操作，网络操作，反射使用的对象
 * 9. 在 finalize 队列中的对象
 * 10. 常量池中的对象，如字符串池引用的对象
 *
 * 对于上面各种 GcRoot，我们准备了不同的遍历方法:
 * 1. 由于现在没有类卸载机制，因此不需要考虑
 * 2. 通过在栈上分配引用表实现，jobject 实际上是指向这些引用的指针
 * 3. 通过 new gc_root 实现，jobject 此时是指向 gc_root 的指针
 * 4. 在 jenv 中保存一个 gc_root 引用，指向创建的 java.lang.Thread 对象
 * 5. 执行锁相关的指令 entermonitor, exitmonitor 时，该对象一定是栈顶元素，不需要额外考虑
 * 6. java 栈帧记录了局部变量表/操作数栈中哪个位置存的是引用
 * 7. 由类加载器负责将存放引用的地址添加到 RcRoot 集合
 * 8. todo
 * 9. 由 gc_thread 维护。每次 gc 结束后会将有必要执行 finalize 函数的对象添加到队列
 * 10. 全局常量池只有一个 (jvm::string)，不直接保存引用而是保存 gc_root 对象
 */

struct gc_root: public gc_ref_base
{
    using ref_set_type = concurrent_set<gc_root*>;

    using static_field_set_type = concurrent_set<jref*>;

    /**
     * 全局 GcRoot 池。所有的 GcRoot 实例都保存在这里，以便 gc 线程遍历
     */
    static ref_set_type ref_pool;

    /**
     * 静态字段池。所有类的所有静态引用类型的字段都会存放在这里，由类加载器
     * 负责维护这个大集合。集合内存储的类型当然也可以是 jfield*，但考虑到
     * 虚拟机目前不支持类卸载，每个静态字段的地址都是确定的。为了避免多余的内存寻址，
     * 我们将其设置成 jref*，这样直接取对象就得到了 jref，而不是 jfield.get()
     */
    static static_field_set_type static_field_pool;

    gc_root(jref ref = nullptr) noexcept:
            gc_ref_base(ref)
    {
        ref_pool.add(this);
    }

    gc_root(const gc_root &o) noexcept:
            gc_ref_base(o)
    {
        ref_pool.add(this);
    }

    gc_root(gc_root &&o) noexcept:
            gc_ref_base(o)
    {
        ref_pool.add(this);
    }

    ~gc_root() noexcept
    {
        ref_pool.remove(this);
    }

    jref &original() noexcept { return const_cast<jref&>(m_ptr); /* return *this; */ }

    [[nodiscard]]
    bool owner() const noexcept override { return true; }

    gc_root& operator=(jref ref) noexcept { reset(ref); return *this; }
};




} /* namespace javsvm */


#endif //JAVSVM_GC_ROOT_H
