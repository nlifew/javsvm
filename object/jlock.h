

#ifndef JAVSVM_JLOCK_H
#define JAVSVM_JLOCK_H

#include <mutex>
#include <condition_variable>

namespace javsvm
{

class jlock
{
private:
    std::mutex m_mutex;
    std::condition_variable m_cond;
    volatile int m_owner_thread_id = -1;
    volatile int m_recursive_count = 0;

public:
    int depth() const noexcept { return (m_recursive_count); }

    jlock() = default;
    ~jlock() = default;
    jlock(const jlock&) = delete;
    jlock& operator=(const jlock&) = delete;

    void lock();

    void unlock();

    void wait(long time = 0);

    void notify();

    void notify_all();
 };
  
} // namespace javsvm


#endif 