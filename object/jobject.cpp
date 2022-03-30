

#include "jobject.h"
#include "../concurrent/pool.h"
#include "../vm/jvm.h"

#include <mutex>
#include <condition_variable>

#define assert_x(x) assert(x) // if (! (x)) { LOGE("assert: %d\n", __LINE__); exit(1); }


using namespace javsvm;


struct T
{
    int operator()() const noexcept
    {
        return jvm::get().env().thread_id;
    }
};


bool lock_event_t::is_locked() const noexcept
{
    auto id = T()();
    bool ok = recursive_count > 0 && owner_thread_id == id;
    if_unlikely(! ok) {
        LOGE("locked: 所有者 %d, 递归数 %d，线程 %d\n", owner_thread_id, recursive_count, id);
    }
    return ok;
}


bool lock_event_t::try_lock() noexcept
{
    const int my_thread_id = T()();
    if (my_thread_id == owner_thread_id) {
        assert(recursive_count > 0);
        recursive_count ++;
        return true;
    }
    if (! mutex.try_lock()) {
        return false;
    }
    recursive_count = 1;
    owner_thread_id = my_thread_id;
    return true;
}


void lock_event_t::lock() noexcept
{
    const int my_thread_id = T()();
    if (my_thread_id == owner_thread_id) {
        assert(recursive_count > 0);
        recursive_count ++;
        return;
    }
    mutex.lock();

    recursive_count = 1;
    owner_thread_id = my_thread_id;
}


void lock_event_t::unlock() noexcept
{
    assert_x(is_locked());

    recursive_count --;
    if (recursive_count == 0) {
        owner_thread_id = INVALID_THREAD_ID;
        mutex.unlock();
    }
}


void lock_event_t::wait(long milliseconds) noexcept
{
    assert_x(is_locked());

    // 保护现场
    std::atomic<uint64_t> defence(0);
    defence |= ((uint64_t) owner_thread_id) << 32;
    defence |= recursive_count;

    std::unique_lock lck(mutex, std::adopt_lock);
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


void lock_event_t::notify_one() noexcept
{
    assert_x(is_locked());
    cond.notify_one();
}


void lock_event_t::notify_all() noexcept
{
    assert_x(is_locked());
    cond.notify_all();
}



static linked_pool<lock_event_t> lock_pool(64);

//static constexpr int COUNT_BIT = 15;
//static constexpr int COUNT_MASK = (1 << COUNT_BIT) - 1;


struct flag_t
{
    static constexpr int LOCK_COUNT_MAX = 1 << 15;

    enum lock_t
    {
        no_lock = 0,
        locked = 15,
    };

    lock_t lock_type;

    size_t object_size;

    int hash_code;


    lock_event_t *lock;
    int lock_count;

    flag_t() noexcept = default;

    flag_t(const flag_t&) = default;

    flag_t& operator=(const flag_t &) = default;

    flag_t(int64_t value) noexcept
    {
        operator=(value);
    }

    flag_t& operator=(int64_t value) noexcept
    {
        lock_type = (lock_t) (value & 15);

        lock_count = (int) (0x7FFF & (value >> 4));
        lock = (lock_event_t *) ((value >> 15) & ~15LL);

        hash_code = (int) (0xFFFFFF & (value >> 5));
        object_size = value >> 29;

        return *this;
    }

    [[nodiscard]]
    int64_t value() const noexcept
    {
        int64_t val = 0;

        if (lock_type == locked) {
            val |= ((int64_t) lock) << 15;
            val |= (0x7FFF & lock_count) << 4;
            val |= locked;
            return val;
        }
        if (lock_type == no_lock) {
            val |= ((int64_t) object_size) << 29;
            val |= (0xFFFFFF & hash_code) << 5;
            val |= no_lock;
            return val;
        }
        return 0;
    }
};


jobject::jobject(size_t size) noexcept
{
    auto hash = (uint64_t) this;
    hash |= hash >> 32;

    flag_t flag {};
    flag.lock_type = flag_t::no_lock;
    flag.hash_code = (int) hash;
    flag.object_size = size;

    m_flag.store(flag.value());
}

int64_t jobject::magic() noexcept
{
    flag_t flag {};

    for (;;) {
        int64_t old_value;
        flag = old_value = m_flag.load();

        if (flag.lock_type == flag_t::no_lock) {
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

        assert_x(flag.lock_type == flag_t::locked);
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
    return flag.hash_code;
}

size_t jobject::size() noexcept
{
    flag_t flag = magic();
    return flag.object_size;
}



lock_event_t *jobject::lock_internal() noexcept
{
    flag_t flag {};

    for (;;) {
        int64_t old_value;
        flag = old_value = m_flag.load();

        if (flag.lock_type == flag_t::locked) {
            assert_x(flag.lock != nullptr);
            assert_x(flag.lock_count > 0 && flag.lock_count < flag_t::LOCK_COUNT_MAX);

            flag.lock_count ++;
        }
        else {
            flag.lock_type = flag_t::locked;
            flag.lock_count = 1;
            flag.lock = &lock_pool.obtain();
            flag.lock->magic = old_value;

            // 正常的情况: 指针高 16 位都是 0 或都是 1，低 4 位都是 0
            auto check = ((uint64_t) flag.lock) >> 48;
            assert_x(check == 0 || check == 0xFFFF);

            check = (uint64_t) flag.lock;
            assert_x((check & 15) == 0);
        }

        // 尝试 CAS 操作更新 flag. 如果成功，直接返回；否则重试
        if (m_flag.compare_exchange_strong(old_value, flag.value())) {
            break;
        }
        if (flag.lock_count == 1) {
            lock_pool.recycle(*flag.lock);
        }
    }

    assert_x(flag.lock_type == flag_t::locked);
    assert_x(flag.lock != nullptr);
    assert_x(flag.lock_count > 0);

    return flag.lock;
}



int jobject::lock() noexcept
{
    lock_internal()->lock();
    return 0;
}


int jobject::unlock() noexcept
{
    for (;;) {
        int64_t old_value;
        flag_t flag = old_value = m_flag.load();

        if_unlikely(flag.lock_type != flag_t::locked || ! flag.lock->is_locked()) {
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



int jobject::wait(long time_millis) noexcept
{
    // 检查当前线程是否已经拥有了锁
    flag_t flag = m_flag.load();

    if_unlikely(flag.lock_type != flag_t::locked || ! flag.lock->is_locked()) {
        return -1;
    }
    flag.lock->wait(time_millis);
    return 0;
}

int jobject::notify_one() noexcept
{
    // 检查当前线程是否已经拥有了锁
    flag_t flag = m_flag.load();

    if_unlikely(flag.lock_type != flag_t::locked || ! flag.lock->is_locked()) {
        return -1;
    }
    flag.lock->notify_one();
    return 0;
}

int jobject::notify_all() noexcept
{
    // 检查当前线程是否已经拥有了锁
    flag_t flag = m_flag.load();

    if_unlikely(flag.lock_type != flag_t::locked || ! flag.lock->is_locked()) {
        return -1;
    }
    flag.lock->notify_all();
    return 0;
}