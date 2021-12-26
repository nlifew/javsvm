//
// Created by edz on 2021/11/14.
//

#include "recursive_lock.h"


using namespace javsvm;

using thread_id = recursive_lock::thread_id;
static constexpr thread_id INVALID_THREAD_ID = -1;


static inline thread_id my_thread_id() noexcept
{
    auto id = INVALID_THREAD_ID;
    pthread_threadid_np(nullptr, &id);
    return id;
}


recursive_lock::recursive_lock() noexcept:
    m_owner_thread_id(INVALID_THREAD_ID),
    m_recursive_count(0)
{
}



void recursive_lock::lock_shared() noexcept
{
    auto id = my_thread_id();
    if (id == m_owner_thread_id) {
        assert(m_recursive_count > 0);
        m_recursive_count ++;
        return;
    }
    m_mutex.lock_shared();
}

bool recursive_lock::try_lock_shared() noexcept
{
    auto id = my_thread_id();
    if (id == m_owner_thread_id) {
        assert(m_recursive_count > 0);
        m_recursive_count ++;
        return true;
    }
    return m_mutex.try_lock_shared();
}


void recursive_lock::unlock_shared() noexcept
{
    auto id = my_thread_id();
    if (id == m_owner_thread_id) {
        m_recursive_count --;
        assert(m_recursive_count > 0);
        return;
    }
    m_mutex.unlock_shared();
}


void recursive_lock::lock() noexcept
{
    auto id = my_thread_id();
    if (id == m_owner_thread_id) {
        m_recursive_count ++;
        return;
    }
    m_mutex.lock();
    assert(m_recursive_count == 0);

    m_owner_thread_id = id;
    m_recursive_count = 1;
}


bool recursive_lock::try_lock() noexcept
{
    auto id = my_thread_id();
    if (id == m_owner_thread_id) {
        m_recursive_count ++;
        return true;
    }
    if (m_mutex.try_lock()) {
        assert(m_recursive_count == 0);
        m_owner_thread_id = id;
        m_recursive_count = 1;
        return true;
    }
    return false;
}

void recursive_lock::unlock() noexcept
{
    assert(m_recursive_count > 0);
    m_recursive_count --;
    if (m_recursive_count == 0) {
        m_owner_thread_id = INVALID_THREAD_ID;
        m_mutex.unlock();
    }
}