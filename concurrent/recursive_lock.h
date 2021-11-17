
#ifndef JAVSVM_CON_RESCURSIVE_LOCK
#define JAVSVM_CON_RESCURSIVE_LOCK


#include <pthread.h>

namespace javsvm
{

class recursive_lock
{
private:
    pthread_mutex_t m_lock;
    pthread_cond_t m_cond;
    volatile int m_reader_count;
public:
    recursive_lock() noexcept;

    ~recursive_lock() noexcept;

    recursive_lock(const recursive_lock&) = delete;
    recursive_lock& operator=(const recursive_lock&) = delete;

    void lock_shared() noexcept;

    bool try_lock_shared() noexcept;

    void unlock_shared() noexcept;

    void lock() noexcept;

    bool try_lock() noexcept;

    void unlock() noexcept;
};

}

#endif 