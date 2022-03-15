

#include "jobject.h"
#include "../concurrent/pool.h"
#include "../vm/jvm.h"

#include <mutex>
#include <condition_variable>

#define assert_x(x) assert(x) // if (! (x)) { LOGE("assert: %d\n", __LINE__); exit(1); }


using namespace javsvm;


template <typename T>
struct lock_event
{
    static constexpr int INVALID_THREAD_ID = -1;

    std::condition_variable cond;
    std::mutex mutex;
    volatile int owner_thread_id = INVALID_THREAD_ID;
    volatile int recursive_count = 0;

    volatile int64_t magic = 0;

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
        return jvm::get().env().thread_id;
    }
};

using lock_event_t = lock_event<thread_id_provider>;

static linked_pool<lock_event_t> lock_pool(64);

//static constexpr int COUNT_BIT = 15;
//static constexpr int COUNT_MASK = (1 << COUNT_BIT) - 1;

struct flag_t
{
    static constexpr int LOCK_COUNT_MAX = 1 << 15;

    enum ctl_t
    {
        ctl_no_lock = 0,
        ctl_locked = 15,
    };

    ctl_t ctl = ctl_no_lock;
    int lock_count = 0;
    lock_event_t *lock = nullptr;

    int hash = 0;
    int size = 0;

    flag_t() noexcept = default;

    flag_t(int64_t val) noexcept
    {
        operator=(val);
    }

    inline flag_t& operator=(int64_t val) noexcept
    {
        ctl = (ctl_t) (val & 15);

        lock_count = (int) ((val >> 4) & 0x7FFF);
        lock = (lock_event_t *) ((val >> 15) & ~15LL);

        hash = (int) (val >> 32);
        size = 0x0FFFFFFF & (int) (val >> 4);
        return *this;
    }

    [[nodiscard]]
    inline int64_t value() const noexcept
    {
        if (ctl == ctl_locked) {
            int64_t val = ((int64_t) lock) << 15;
            val |= lock_count << 4;
            val |= ctl;
            return val;
        }
        if (ctl == ctl_no_lock) {
            int64_t val = ((int64_t) hash) << 32;
            val |= size << 4;
            val |= ctl;
            return val;
        }
        return 0;
    }
};


jobject::jobject(int size) noexcept
{
    auto hash = (uint64_t) this;
    hash |= hash >> 32;

    flag_t flag;
    flag.ctl = flag_t::ctl_no_lock;
    flag.hash = (int) hash;
    flag.size = size;

    m_flag.store(flag.value());
}

int64_t jobject::magic() noexcept
{
    flag_t flag;

    for (;;) {
        int64_t old_value;
        flag = old_value = m_flag.load();

        if (flag.ctl == flag_t::ctl_no_lock) {
            return old_value;
        }

        assert_x(flag.lock != nullptr);
        assert_x(flag.lock_count > 0 && flag.lock_count < flag_t::LOCK_COUNT_MAX);

        flag.lock_count ++;
        if (m_flag.compare_exchange_strong(old_value, flag.value())) {
            break;
        }
    }

    auto magic = flag.lock->magic;

    for (;;) {
        int64_t old_value;
        flag = old_value = m_flag.load();

        assert_x(flag.ctl == flag_t::ctl_locked);
        assert_x(flag.lock != nullptr);
        assert_x(flag.lock_count >= 1);

        int64_t new_value;

        if (flag.lock_count == 1) {
            new_value = flag.lock->magic;
        }
        else {
            flag.lock_count --;
            new_value = flag.value();
            flag.lock_count ++; // 防止 flag.lock_count == 2 时下边误判
        }
        // CAS 操作更新 flag
        if (m_flag.compare_exchange_strong(old_value, new_value)) {
            if (flag.lock_count == 1) {
                lock_pool.recycle(*flag.lock);
            }
            break;
        }
    }
    return magic;
}


int jobject::hash_code() noexcept
{
    flag_t flag = magic();
    return flag.hash;
}

int jobject::size() noexcept
{
    flag_t flag = magic();
    return flag.size;
}

int jobject::lock() noexcept
{
    flag_t new_flag;

    for (;;) {
        int64_t old_value;
        flag_t old_flag = old_value = m_flag.load();

        if (old_flag.ctl == flag_t::ctl_locked) {
            assert_x(old_flag.lock != nullptr);
            assert_x(old_flag.lock_count > 0 && old_flag.lock_count < flag_t::LOCK_COUNT_MAX);

            new_flag = old_flag;
            new_flag.lock_count ++;
        }
        else {
            new_flag.ctl = flag_t::ctl_locked;
            new_flag.lock_count = 1;
            new_flag.lock = &lock_pool.obtain();
            new_flag.lock->magic = old_value;

            // 正常的情况: 指针高 16 位都是 0 或都是 1，低 4 位都是 0
            auto check = ((int64_t) new_flag.lock) >> 48;
            assert_x(check == 0 || check == 0xFFFF);

            check = (int64_t) new_flag.lock;
            assert_x((check & 15) == 0);
        }

        // 尝试 CAS 操作更新 flag. 如果成功，直接返回；否则重试
        if (m_flag.compare_exchange_strong(old_value, new_flag.value())) {
            break;
        }
        if (old_flag.ctl == flag_t::ctl_no_lock) {
            lock_pool.recycle(*new_flag.lock);
        }
    }

    assert_x(new_flag.ctl == flag_t::ctl_locked);
    assert_x(new_flag.lock != nullptr);
    assert_x(new_flag.lock_count > 0);

    new_flag.lock->lock();
    return 0;
}


int jobject::unlock() noexcept
{
    for (;;) {
        int64_t old_value;
        flag_t flag = old_value = m_flag.load();

        if (flag.ctl != flag_t::ctl_locked || ! flag.lock->am_i_locked()) {
            // 说明之前没有锁住，是异常状态
            return -1;
        }
        assert_x(flag.lock_count >= 1);

        // 当 count 为 1，说明没有任何线程尝试获取锁，需要释放掉 lock_event
        int64_t new_value;

        if (flag.lock_count == 1) {
            new_value = flag.lock->magic;
        }
        else {
            flag.lock_count --;
            new_value = flag.value();
            flag.lock_count ++;
        }

        // CAS 操作更新 flag
        if (m_flag.compare_exchange_strong(old_value, new_value)) {
            flag.lock->unlock();
            if (flag.lock_count == 1) {
                lock_pool.recycle(*flag.lock);
            }
            break;
        }
    }
    return 0;
}


static inline lock_event_t *am_i_locked(int64_t value) noexcept
{
    flag_t flag = value;

    if (flag.ctl != flag_t::ctl_locked || ! flag.lock->am_i_locked()) {
        return nullptr;
    }
    assert_x(flag.lock_count >= 1);
    return flag.lock;
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