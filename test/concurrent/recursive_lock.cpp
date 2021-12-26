//
// Created by edz on 2021/12/26.
//
#include <thread>
#include "concurrent/recursive_lock.h"

using javsvm::recursive_lock;


int main()
{
    recursive_lock lock;
    assert(lock.try_lock_shared());
    assert(lock.try_lock_shared());
    assert(! lock.try_lock());
    lock.unlock_shared();
    assert(! lock.try_lock());
    lock.unlock_shared();
    assert(lock.try_lock());

    assert(lock.try_lock_shared());
    assert(lock.try_lock_shared());
    assert(lock.try_lock());
    assert(lock.try_lock());

    lock.unlock_shared();
    lock.unlock_shared();
    lock.unlock();
    lock.unlock();

    std::thread t([&lock] {
        assert(! lock.try_lock());
        assert(! lock.try_lock_shared());
        assert(! lock.try_lock());
    });
    t.join();

    lock.unlock();

    std::thread t1([&lock] {
        assert(lock.try_lock_shared());
        std::this_thread::sleep_for(std::chrono::seconds(5));
        lock.unlock_shared();
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    assert(! lock.try_lock());
    t1.join();

    return 0;
}