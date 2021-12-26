

#include "jobject.h"
#include "jlock.h"
#include "../concurrent/pool.h"

#include <thread>

using namespace javsvm;


static linked_pool<jlock> global_lock_pool(32);

#define ENTER_BARRIER \
    bool _expect_barrier = false; \
    while (!m_barrier.compare_exchange_weak(_expect_barrier, true)) { \
        _expect_barrier = false;  \
        std::this_thread::yield(); \
    }

#define EXIT_BARRIER \
    m_barrier.store(false);


void jobject::lock()
{
    ENTER_BARRIER

    if (m_owner_thread_count++ == 0) {
        m_lock = &global_lock_pool.obtain();
    }

    EXIT_BARRIER

    const_cast<jlock*>(m_lock)->lock();
}


void jobject::unlock()
{
    const_cast<jlock*>(m_lock)->unlock();

    ENTER_BARRIER
    if (--m_owner_thread_count == 0) {
        global_lock_pool.recycle((const jlock &) *m_lock);
        m_lock = nullptr;
    }
    EXIT_BARRIER
}

void jobject::wait(long time_millis)
{
    const_cast<jlock*>(m_lock)->wait(time_millis);
}

void jobject::notify()
{
    const_cast<jlock*>(m_lock)->notify();
}

void jobject::notify_all()
{
    const_cast<jlock*>(m_lock)->notify_all();
}