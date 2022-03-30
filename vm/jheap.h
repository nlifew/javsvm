

#ifndef JAVSVM_JHEAP_H
#define JAVSVM_JHEAP_H

#include <atomic>
#include <vector>

#include "../gc/gc_thread.h"
#include "../utils/global.h"
#include "../object/jclass.h"
#include "../concurrent/atomic_lock.h"
#include "../concurrent/concurrent_map.h"


namespace javsvm {

struct jobject;



class jheap {
private:
    friend class gc_thread;

    size_t m_max_cap = 0;
    size_t m_cur_cap = 0;
    char *m_buff = nullptr;

    std::atomic<size_t> m_floating_ptr { 0 };
public:
    gc_thread m_gc_thread;

    /**
     * 使用指针碰撞法快速分配内存
     * @param size 需要分配的内存大小，可以不对齐
     * @return 成功返回指针，同时 m_floating_ptr 增加 size 大小;
     * 失败返回 nullptr, m_floating_ptr 不变
     */
    jobject *fast_alloc(int size) noexcept;


    /**
     * 尝试重新分配堆内存大小
     * 如果堆已经无法再增大，或者能增大的部分少于 size，则失败
     * NOTE: 此函数只用于扩容，而且没有线程同步措施，因此要求调用者自己整理内存和处理多线程问题
     *
     * @param size 如果成功，分配后的堆最少增加了 size 大小。比如现在堆大小是 32M,
     *              size 是 16M, 如果成功，堆最少为 48M
     * @return 成功返回 0，否则返回 -1
     */
    int realloc_size(size_t size);



    /**
     * 如果某个类重写了 finalize() 函数，分配时
     * 会将指针记录在这里面，用于 gc 时将其放在 finalize 队列
     */
    std::vector<jref> m_finalize_object;

    /**
     * m_finalize_object 字段使用的锁
     */
    atomic_lock m_finalize_lock;

    /**
     * 将类和对象绑定，比如设置 klass 指针等
     */
     inline jref bind(jobject *dst, jclass *src);

public:
    static constexpr int DEFAULT_MIN_CAPACITY = 4 * 1024 * 1024; // aka 4MB
    static constexpr int DEFAULT_MAX_CAPACITY = 64 * 1024 * 1024; // aka 64M

    /**
     * 开辟一个新的堆
     * @param min_cap 最小内存，堆初始化时即分配此大小
     * @param max_cap 堆的最大内存，当堆已经达到最大大小，又无法通过 gc
     * 回收足够的内存时，就会抛出内存溢出错误(OutOfMemoryError)
     */
    explicit jheap(
            size_t min_cap = DEFAULT_MIN_CAPACITY,
            size_t max_cap = DEFAULT_MAX_CAPACITY) noexcept;

    jheap(const jheap &) = delete;

    jheap &operator=(const jheap &) = delete;

    ~jheap() noexcept;

    /**
     * 尝试分配一块内存用来存放对象
     * @param size 对象的大小，不包括对象头和填充数据，因此不需要调用者手动对齐
     * @return 成功返回 jref，失败返回 nullptr
     */
    jref alloc(jclass *klass, size_t size) noexcept;

    jref alloc(jclass *klass) noexcept { return alloc(klass, klass->object_size); }

    static bool equals(jref p, jref q) noexcept {
        return cast(p) == cast(q);
    }

    static inline jobject *cast(jref ref) noexcept {
        return (jobject *) (((uint64_t) ref) & ~R_MSK);
    }

    static constexpr uint64_t R_MSK =   3;
    static constexpr uint64_t R_STR =   0;
    static constexpr uint64_t R_SFT =   1;
    static constexpr uint64_t R_WEK =   2;
    static constexpr uint64_t R_PHA =   3;
};
}
#endif