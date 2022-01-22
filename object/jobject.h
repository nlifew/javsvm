

#ifndef JAVSVM_JOBJECT_H
#define JAVSVM_JOBJECT_H

#include <atomic>

namespace javsvm
{

struct jclass;

struct jobject
{
private:
    /**
     * 64 位的标志位，在不同的情况下有不同的意义（取决于低 4 位）
     * 当最低 4 位为以下情况时:
     *
     * 1. 0x00 (0b0000)
     * 即大多数普通对象的状态。高 32 位表示 hashCode 值，低 5 - 8 位为 gc 年龄
     *
     * 2. 0xFF (0b1111)
     * 表示当前这个对象正在被锁住。最高位即符号位表示 lock_event 指针的前 16 位填充数据，接下来的 44 位为指针数据
     * 15 位表示尝试等待锁的线程数，4 位固定为 0xFF
     */
    std::atomic<int64_t> m_flag;
public:

    jobject() noexcept;

    jclass *klass = nullptr;
    char values[0];

    /**
     * 获取该对象的 hashCode 值
     * hashCode 在对象创建的时候就已经确定
     */
    int hash_code() noexcept;

    /**
     * 尝试获取该对象的锁
     */
    int lock() noexcept;

    /**
     * 尝试释放对象的锁
     */
    int unlock() noexcept;

    /**
     * 释放锁的同时挂起线程。调用者需要事先锁住该对象，否则会失败
     * @param time_millis 超时毫秒数。超过这个值会自动唤醒。为 0 时表示无限等待
     */
    int wait(long time_millis = 0) noexcept;

    /**
     * 唤醒某一个正在等待此对象的线程。调用者需要事先锁住该对象，否则会失败
     */
    int notify_one() noexcept;

    /**
     * 唤醒正在等待此对象的所有线程。调用者需要事先锁住该对象，否则会失败
     */
    int notify_all() noexcept;
};

} // namespace javsvm


#endif
