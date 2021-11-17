

#ifndef JAVSVM_CON_ATOMIC_LOCK
#define JAVSVM_CON_ATOMIC_LOCK

#include <atomic>

namespace javsvm 
{

class atomic_lock
{

private:
    std::atomic<int> m_reader_count;
    std::atomic<bool> m_has_writer;

public:
    atomic_lock(): m_reader_count(0), m_has_writer(false)
    {
    }

    atomic_lock(const atomic_lock &) = delete;
    atomic_lock &operator=(const atomic_lock &) = delete;

    void lock();
    bool try_lock();
    void unlock();

    void lock_shared();
    // bool try_lock_shared();
    void unlock_shared();
};

};

#endif 