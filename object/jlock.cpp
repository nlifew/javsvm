

#include "jlock.h"

#include <new>
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
    // 在实际调用 wait 之前，我们需要暂时保存一下当前锁的递归深度
    // 因为 wait() 函数会在挂起线程的同时释放掉锁。其它线程获取锁之后
    // 会覆盖掉递归深度。
    std::atomic<u8> backup;
    backup.store((((u8) m_owner_thread_id) << 32) | m_recursive_count);

    m_recursive_count = 0;
    m_owner_thread_id = -1;

    std::unique_lock guard(m_mutex, std::adopt_lock);

    if (time <= 0) {
        m_cond.wait(guard);
    }
    else {
        m_cond.wait_for(guard, std::chrono::milliseconds(time));
    }
    // 恢复备份
    const u8 backup_value = backup.load();
    m_owner_thread_id = (int) (backup_value >> 32);
    m_recursive_count = (int) backup_value;

    // 防止 unique_lock 的析构函数将锁释放掉
    guard.release();
}


void jlock::notify()
{
    m_cond.notify_one();
}

void jlock::notify_all()
{
    m_cond.notify_all();
}