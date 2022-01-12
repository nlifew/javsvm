

#ifndef JAVSVM_JOBJECT_H
#define JAVSVM_JOBJECT_H

#include <atomic>
#include "../utils/global.h"
//#include "jlock.h"

namespace javsvm
{

struct jclass;

struct jobject
{
private:
    std::atomic<int64_t> m_flag;
public:
    jclass *klass = nullptr;
    char values[0];

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
