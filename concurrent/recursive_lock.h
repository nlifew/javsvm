
#ifndef JAVSVM_CON_RECURSIVE_LOCK
#define JAVSVM_CON_RECURSIVE_LOCK

#include <cstdint>
#include <shared_mutex>

namespace javsvm
{

/**
 * 支持递归使用和锁降级的读写锁
 */

class recursive_lock
{
public:
    using thread_id = uint64_t;
private:
    std::shared_mutex m_mutex;
    thread_id m_owner_thread_id;
    volatile int m_recursive_count;

public:
    recursive_lock() noexcept;

    ~recursive_lock() noexcept = default;

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