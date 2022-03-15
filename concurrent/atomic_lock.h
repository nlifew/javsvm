
#ifndef JAVSVM_CON_ATOMIC_LOCK_H
#define JAVSVM_CON_ATOMIC_LOCK_H

#include <atomic>
#include <thread>

namespace javsvm
{

class atomic_lock
{
private:
    std::atomic<int> m_flag = { 0 };
public:
    static unsigned CONCURRENT_THREAD;

    void lock() noexcept
    {
        int expected = 0;

        // 不断自旋直到获取锁（或者达到指定次数）
        if (CONCURRENT_THREAD > 1) {
            for (int i = 0; i < 1000; ++i) {
                if (m_flag.compare_exchange_strong(expected, 1)) {
                    return;
                }
                std::this_thread::yield();
                expected = 0;
            }
        }

        while (! m_flag.compare_exchange_strong(expected, 1)) {
            std::this_thread::sleep_for(std::chrono::microseconds(10));
            expected = 0;
        }
    }

    void unlock() noexcept
    {
        m_flag = 0;
    }

    bool try_lock() noexcept
    {
        int expected = 0;
        return m_flag.compare_exchange_strong(expected, 1);
    }
};

}


#endif