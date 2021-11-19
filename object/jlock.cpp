

#include "jlock.h"

#include <thread>

#include "../utils/global.h"
#include "../utils/log.h"
#include "../vm/jenv.h"
#include "../vm/jvm.h"

using namespace javsvm;


//static constexpr u8 FLAG_LOCK_POINTER_MASK      =   0xFFFFFFFFFFFF;
//static constexpr u8 FLAG_LOCK_TYPE_MASK         =   ((u8) 7) << 48;
//static constexpr u8 FLAG_LOCK_TYPE_NONE         =   ((u8) 0) << 48;
//static constexpr u8 FLAG_LOCK_TYPE_BIAS         =   ((u8) 1) << 48;
//static constexpr u8 FLAG_LOCK_TYPE_MUTEX        =   ((u8) 1) << 49;
//static constexpr u8 FLAG_LOCK_TYPE_MUTEX_BIAS   =   ((u8) 1) << 50;

void jlock::lock()
{
    const int thread_id = jvm::get().env().thread_id();
    if (thread_id == m_owner_thread_id) {
        m_recursive_count ++;
        return;
    }

    m_mutex.lock();

    m_owner_thread_id = thread_id;
    m_recursive_count = 1;
}

void jlock::unlock()
{
    if (--m_recursive_count == 0) {
        m_owner_thread_id = -1;
        m_mutex.unlock();
    }
}

void jlock::wait(long time)
{
    std::unique_lock guard(m_mutex, std::adopt_lock_t());
    if (time <= 0) {
        m_cond.wait(guard);
    }
    else {
        m_cond.wait_for(guard, std::chrono::milliseconds(time));
    }
}


void jlock::notify()
{
    m_cond.notify_one();
}

void jlock::notify_all()
{
    m_cond.notify_all();
}