//
// Created by edz on 2022/1/29.
//

#ifndef JAVSVM_GC_THREAD_H
#define JAVSVM_GC_THREAD_H

#include "../utils/global.h"

#include <csignal>
#include <vector>
#include <thread>
#include <mutex>
#include <list>
#include <condition_variable>

namespace javsvm
{

class jobject;
class jfield;
class jheap;

class gc_thread
{
private:
    /**
     * 一个真正的线程对象
     * gc_thread 初始化时，并不会立即创建一个线程，而是在第一次 gc 前创建
     */
    volatile std::thread *m_native_thread = nullptr;
    char m_native_thread_buff[sizeof(std::thread)]{};

//    enum status_t
//    {
//        INIT = 0,
//        FREE,
//        BUSY,
//        CANCEL,
//    };
//    status_t m_status = INIT;

    /**
     * 锁
     */
    std::mutex m_mutex;

    /**
     * 监听 STW 用的
     */
    std::condition_variable m_stw_cond;

    /**
     * 监听 gc 空闲/工作中用的
     */
    std::condition_variable m_trap_cond;

    /**
     * 处于安全状态的 java 线程数
     * 当这个数字和 jvm 存储的总线程数相同时才会进行 gc
     */
    volatile int m_blocked_threads_count = 0;

    /**
     * 保存的堆指针。通过 attach() 函数建立联系
     */
    jheap *m_heap = nullptr;

    [[nodiscard]]
    bool is_the_world_stopped() const noexcept;

    /**
     * gc 线程的入口点
     */
    [[noreturn]]
    void run() noexcept;

    /**
     * 收集 GcRoot + 追踪引用链
     */
    void mark() noexcept;

    /**
     * 使用染色法递归式追踪引用链
     */
    void trace_and_mark(jobject *obj) noexcept;

    /**
     * 针对弱引用/软引用/虚引用的特殊处理
     * 比如移动对象到引用队列
     */
    void special_for_java_lang_ref(jref ref, int offset) noexcept;


    /**
     * 整理堆内存
     * 包括: 重新调整堆的大小，移动对象到堆的一端，更新隐藏在各处的引用，对象脱染色等
     */
    void compact() noexcept;


    void trace_and_restore(jobject **mapping, jref &ref) noexcept;

    /**
     * 用来保存此次 gc 过程中所有的存活对象
     */
    std::vector<jobject*> m_alive_objects;

    /**
     * 上次 gc 时收集到的根节点数量，用于对本次根节点数做个预测，减少无谓的扩容步骤
     */
    size_t m_last_gc_root_num = 64;

    /**
     * 保存此次 gc 过程中所有存活对象占用的空间大小
     * 单位是字节
     */
    size_t m_alive_memory = 0;

    /**
     * 需要执行 finalize() 的对象集合
     */
    std::list<jref> m_finalize_queue;


    struct sigaction m_sigsegv_backup{};

    void register_sigsegv_handler() noexcept;

    void unregister_sigsegv_handler() noexcept;

    void handle_sigsegv() noexcept;

public:
//    static void on_thread_detach() noexcept;


    gc_thread() noexcept;

    ~gc_thread() noexcept;

    void gc() noexcept;

    void attach(jheap *heap) noexcept
    {
        assert(m_heap == nullptr);
        m_heap = heap;
    }

    inline void enter_safety_area() noexcept;

    inline void exit_safety_area() noexcept;
};


} // namespace javsvm

#endif //JAVSVM_GC_THREAD_H
