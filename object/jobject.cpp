

#include "jobject.h"
#include "../concurrent/pool.h"
#include "../vm/jvm.h"

#include <mutex>
#include <condition_variable>

#define assert_x(x) assert(x)


using namespace javsvm;

#define USE_RECURSIVE_LOCK 1

template <typename T>
struct lock_event
{
    static constexpr int INVALID_THREAD_ID = -1;

    std::condition_variable cond;
    std::mutex mutex;
    volatile int owner_thread_id = INVALID_THREAD_ID;
    volatile int recursive_count = 0;

    explicit lock_event() noexcept = default;

    lock_event(const lock_event&) = delete;
    lock_event(lock_event &&) = delete;
    lock_event& operator=(const lock_event &) = delete;
    lock_event& operator=(lock_event&&) = delete;

    [[nodiscard]]
    bool am_i_locked() const noexcept
    {
        auto id = T()();
        bool ok = recursive_count > 0 && owner_thread_id == id;
        if (! ok) {
            LOGE("am_i_locked: 所有者 %d, 递归数 %d，线程 %d\n", owner_thread_id, recursive_count, id);
        }
        return ok;
    }

    void lock() noexcept
    {
        const int my_thread_id = T()();
        if (my_thread_id == owner_thread_id) {
            assert_x(recursive_count > 0);
            recursive_count ++;
            return;
        }
        mutex.lock();
//        std::atomic_signal_fence(std::memory_order_seq_cst);

        recursive_count = 1;
        owner_thread_id = my_thread_id;
    }

    void unlock() noexcept
    {
        assert_x(am_i_locked());

        recursive_count --;
        if (recursive_count == 0) {
            owner_thread_id = INVALID_THREAD_ID;
//            std::atomic_signal_fence(std::memory_order_seq_cst);
            mutex.unlock();
        }
    }

    void wait(long milliseconds) noexcept
    {
        assert_x(am_i_locked());

        // 保护现场
        std::atomic<uint64_t> defence(0);
        defence |= ((uint64_t) owner_thread_id) << 32;
        defence |= recursive_count;

        std::unique_lock lck(mutex, std::adopt_lock_t());
        if (milliseconds <= 0) {
            cond.wait(lck);
        }
        else {
            cond.wait_for(lck, std::chrono::milliseconds(milliseconds));
        }
        // 恢复现场
        recursive_count = (volatile int) (defence);
        owner_thread_id = (volatile int) (defence >> 32);

        // 非常重要 !!!
        // 一定要调用 release() 把 unique_lock 的所有权释放了，否则 unique_lock 析构
        // 时会把锁释放掉 !!!
        lck.release();
    }

    void notify_one() noexcept
    {
        assert_x(am_i_locked());
        cond.notify_one();
    }

    void notify_all() noexcept
    {
        assert_x(am_i_locked());
        cond.notify_all();
    }
};


struct thread_id_provider
{
    int operator()() const noexcept
    {
        return jvm::get().env().thread_id();
    }
};

using lock_event_t = lock_event<thread_id_provider>;

static linked_pool<lock_event_t> lock_pool(64);

static constexpr int COUNT_BIT = 15;
static constexpr int COUNT_MASK = (1 << COUNT_BIT) - 1;

int jobject::lock() noexcept
{
    int64_t old_value, new_value;
    lock_event_t *lock;
    int count;

    for (;;) {
        old_value = m_flag.load(std::memory_order_acquire);
        lock = (lock_event_t*) (old_value >> COUNT_BIT);
        count = (int) (old_value & COUNT_MASK);

        if (count == 0) {
            assert_x(lock == nullptr);
            lock = &lock_pool.obtain();

            // 检查 lock 指针的前 (COUNT_BIT + 1) 位是不是纯 0 或 纯 1
            auto check = ((uint64_t) lock) >> (64 - COUNT_BIT + 1);
            if (check != 0 && check != COUNT_MASK) {
                LOGE("lock: 指针检查未通过 %p\n", lock);
//                exit(1);
                return -1;
            }
        }

        // 判断 count 有没有发生溢出
        if (count == COUNT_MASK) {
            LOGE("lock: 加锁数溢出 ！！！");
//            exit(1);
            return -1;
        }

        new_value = (((int64_t) lock) << COUNT_BIT) + count + 1;

        // 尝试 CAS 操作更新 flag. 如果成功，直接返回；否则重试
        if (m_flag.compare_exchange_strong(old_value, new_value)) {
            break;
        }
        if (count == 0) {
            lock_pool.recycle(*lock);
        }
    }

    assert_x(lock != nullptr);
    assert_x(count >= 0);

    lock->lock();
    return 0;
}


int jobject::unlock() noexcept
{
    int64_t old_value, new_value;
    lock_event_t *lock;
    int count;

    for (;;) {
        old_value = m_flag.load(std::memory_order_acquire);
        count = (int) (old_value & COUNT_MASK);
        lock = (lock_event_t*) (old_value >> COUNT_BIT);

        if (count == 0 || lock == nullptr || ! lock->am_i_locked()) {
            // 说明之前没有锁住，是异常状态
            return -1;
        }

        // 当 count 为 1，说明没有任何线程尝试获取锁，需要释放掉 lock_event
        new_value = count == 1 ? 0 : (old_value - 1);

        // CAS 操作更新 flag
        if (m_flag.compare_exchange_strong(old_value, new_value)) {
            lock->unlock();
            if (count == 1) {
                lock_pool.recycle(*lock);
            }
            break;
        }
    }
    return 0;
}


static inline lock_event_t *am_i_locked(int64_t value) noexcept
{
    auto lock = (lock_event_t *) (value >> COUNT_BIT);
    auto count = (int) (value & COUNT_MASK);

    if (count == 0 || lock == nullptr || ! lock->am_i_locked()) {
        return nullptr;
    }
    return lock;
}

int jobject::wait(long time_millis) noexcept
{
    // 检查当前线程是否已经拥有了锁
    auto value = m_flag.load(std::memory_order_acquire);
    auto ok = am_i_locked(value);
    if (! ok) {
        return -1;
    }
    ok->wait(time_millis);
    return 0;
}

int jobject::notify_one() noexcept
{
    auto value = m_flag.load(std::memory_order_acquire);
    auto ok = am_i_locked(value);
    if (! ok) {
        return -1;
    }
    ok->notify_one();
    return 0;
}

int jobject::notify_all() noexcept
{
    auto value = m_flag.load(std::memory_order_acquire);
    auto ok = am_i_locked(value);
    if (! ok) {
        return -1;
    }
    ok->notify_all();
    return 0;
}