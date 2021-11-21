

#include "jlock.h"

#include <new>
#include <thread>

#include "../utils/global.h"
#include "../utils/log.h"
#include "../vm/jenv.h"
#include "../vm/jvm.h"

using namespace javsvm;


#if JLOCK_USE_BIASED_LOCK

static constexpr int FLAG_LOCK_TYPE_MASK         =   3;
static constexpr int FLAG_LOCK_TYPE_NONE         =   0;
static constexpr int FLAG_LOCK_TYPE_BIAS         =   1 << 0;
static constexpr int FLAG_LOCK_TYPE_MUTEX        =   1 << 1;


void jlock::lock()
{
    const int thread_id = jvm::get().env().thread_id();

    // 先处理递归获取锁
    if (m_owner_thread_id.load() == thread_id) {
        m_recursive_count ++ ;
        return;
    }

    // 如果锁没有被任何线程持有，快速返回
    int expected_thread_id = -1;
    if (m_owner_thread_id.compare_exchange_strong(expected_thread_id, thread_id)) {
        m_recursive_count = 1;
        m_flag |= FLAG_LOCK_TYPE_BIAS;
        return;
    }

    // 执行到这里说明当前锁被另一个线程持有，锁升级

    std::unique_lock lck(m_mutex);

    expected_thread_id = -1;
    while (! m_owner_thread_id.compare_exchange_weak(expected_thread_id, thread_id)) {
        m_cond.wait(lck);
    }
    // 成功获取到锁
    m_recursive_count = 1;
    m_flag |= FLAG_LOCK_TYPE_MUTEX;

    lck.release(); // 阻止 lck.unlock();
}

void jlock::unlock()
{
    // 递归深度不是 0，只需要降低递归深度即可
    m_recursive_count --;
    if (m_recursive_count != 0) {
        return;
    }
    // 如果发现获取到的是 mutex 锁，需要 m_mutex.unlock()
    if ((m_flag & FLAG_LOCK_TYPE_MASK) == FLAG_LOCK_TYPE_MUTEX) {
        m_flag &= ~FLAG_LOCK_TYPE_MUTEX;
        m_owner_thread_id.store(-1);
        m_mutex.unlock();
        return;
    }

    // 如果获取到的是偏向锁，则要通知其它正在等待的线程
    if ((m_flag & FLAG_LOCK_TYPE_MASK) == FLAG_LOCK_TYPE_BIAS) {
        std::lock_guard lck(m_mutex);

        m_flag &= ~FLAG_LOCK_TYPE_BIAS;
        m_owner_thread_id.store(-1);
        m_cond.notify_all();
        return;
    }
}

void jlock::wait(long time)
{
    // 在实际调用 wait 之前，我们需要暂时保存一下当前锁的递归深度
    // 因为 wait() 函数会在挂起线程的同时释放掉锁。其它线程获取锁之后
    // 会覆盖掉递归深度。

    int flag = m_flag;
    const int recursive_count = m_recursive_count;
    const int owner_thread_id = m_owner_thread_id.load();

    m_flag &= ~FLAG_LOCK_TYPE_MUTEX;

    const int lock_type = m_flag & FLAG_LOCK_TYPE_MASK;
    if (lock_type == FLAG_LOCK_TYPE_MUTEX) {
        std::unique_lock lck(m_mutex, std::adopt_lock);

        m_owner_thread_id.store(-1);
        if (time <= 0) {
            m_cond.wait(lck);
        }
        else {
            m_cond.wait_for(lck, std::chrono::milliseconds(time));
        }
        lck.release();
    }
    else if (lock_type == FLAG_LOCK_TYPE_BIAS) {
        flag &= ~FLAG_LOCK_TYPE_BIAS;
        flag |= FLAG_LOCK_TYPE_MUTEX;

        std::unique_lock lck(m_mutex);

        m_owner_thread_id.store(-1);
        if (time <= 0) {
            m_cond.wait(lck);
        }
        else {
            m_cond.wait_for(lck, std::chrono::milliseconds(time));
        }
        lck.release();
    }
    m_flag = flag;
    m_recursive_count = recursive_count;
    m_owner_thread_id.store(owner_thread_id);
}

void jlock::notify()
{
    m_cond.notify_one();
}

void jlock::notify_all()
{
    m_cond.notify_all();
}

#else
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

#endif