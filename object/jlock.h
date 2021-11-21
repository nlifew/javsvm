

#ifndef JAVSVM_JLOCK_H
#define JAVSVM_JLOCK_H

#include <mutex>
#include <condition_variable>

#define JLOCK_USE_BIASED_LOCK 0

namespace javsvm
{

class jlock
{
private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    volatile int m_recursive_count = 0;

#if JLOCK_USE_BIASED_LOCK
    volatile int m_flag = 0;
    std::atomic<int> m_owner_thread_id;
#else
    volatile int m_owner_thread_id = -1;
#endif
public:
#if JLOCK_USE_BIASED_LOCK
    jlock() noexcept: m_owner_thread_id(-1)
    {
    }
#else
    jlock() = default;
#endif
    ~jlock() = default;
    jlock(const jlock&) = delete;
    jlock& operator=(const jlock&) = delete;

    [[nodiscard]]
    int depth() const noexcept { return m_recursive_count; }

    void lock();

    void unlock();

    void wait(long time = 0);

    void notify();

    void notify_all();
 };
  
} // namespace javsvm


#endif 