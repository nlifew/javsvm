

#include "atomic_lock.h"
#include "../utils/log.h"
#include <cstdlib>
#include <thread>

using namespace javsvm;

void atomic_lock::lock()
{
    // LOGI("atomic_lock: lock()\n");

    // 首先拿到写者锁，阻塞后续的读者和写者
    bool expect = false;
    while (! m_has_writer.compare_exchange_weak(expect, true)) {
        std::this_thread::yield();

        expect = false; // CAS 失败时，expect 结果是未知的，因此必须复位
    }

    // 等待所有的读者完成
    while (m_reader_count.load() != 0) {
        std::this_thread::yield();
    }
}

bool atomic_lock::try_lock()
{
    bool expect = false;
    if (! m_has_writer.compare_exchange_weak(expect, true)) {
        return false;
    }
    while (m_reader_count.load() != 0) {
        std::this_thread::yield();
    }
    return true;
}

void atomic_lock::unlock()
{
    // LOGI("atomic_lock: unlock()\n");
    if (! m_has_writer.exchange(false)) {
        LOGE("duplicated unlock() !!\n");
        exit(1);
    }
}


void atomic_lock::lock_shared()
{
    bool expect = false;
    while (! m_has_writer.compare_exchange_weak(expect, true)) {
        std::this_thread::yield();

        expect = false; // CAS 失败时，expect 结果是未知的，因此必须复位
    }
    m_reader_count ++;
    m_has_writer.store(false);
}

#if 0
bool atomic_lock::try_lock_shared()
{

}
#endif

void atomic_lock::unlock_shared()
{
    if (m_reader_count.operator--(1) < 0) {
        LOGE("duplicated unlock_shared() !!\n");
        exit(1);
    }
}
