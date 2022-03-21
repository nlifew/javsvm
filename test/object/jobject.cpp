//
// Created by edz on 2022/1/12.
//
#include "object/jobject.h"
#include "utils/log.h"
#include "vm/jvm.h"

#include <thread>
#include <deque>

using namespace javsvm;

struct lock_event
{
    std::mutex m_mutex;
    std::condition_variable m_cond;

    int lock() noexcept { m_mutex.lock(); return 0; }

    int unlock() noexcept { m_mutex.unlock(); return 0; }

    int wait(long time = 0) noexcept {
        std::unique_lock lck(m_mutex, std::adopt_lock);
        m_cond.wait(lck);
        lck.release();
        return 0;
    }

    int notify_one() noexcept { m_cond.notify_one(); return 0;}

    int notify_all() noexcept { m_cond.notify_all(); return 0;}
};

jobject m_lock(0);
int hash = 0;
//lock_event m_lock;

std::deque<int> m_queue;


#define N 1000

#ifndef NDEBUG
#define assert_x(x) assert(x)
#else
#define assert_x(x) if (! (x)) exit(1);
#endif

static void product()
{
    const int thread_id = jvm::get().attach().thread_id;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    for (int i = 0; i < N; i ++) {
        assert_x(hash == m_lock.hash_code());
        LOGI("product: 生产 %d\n", i);
        assert_x(m_lock.lock() == 0);
        assert_x(m_lock.lock() == 0);

        while (m_queue.size() >= 10) {
            LOGI("product: 队列已满，等待\n");
            assert_x(m_lock.wait() == 0);
        }
        LOGI("product: 加入队列\n");
        m_queue.push_back(i);

        if (m_queue.size() == 1) {
            LOGI("product: 通知消费者\n");
            assert_x(m_lock.notify_all() == 0);
        }
        assert_x(m_lock.unlock() == 0);
        assert_x(m_lock.unlock() == 0);
    }
}

static void consumer()
{
    const int thread_id = jvm::get().attach().thread_id;
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    for (int i = 0; i < N; i ++) {
        assert_x(hash == m_lock.hash_code());
        assert_x(m_lock.lock() == 0);
        assert_x(m_lock.lock() == 0);

        while (m_queue.empty()) {
            LOGI("consumer: 队列为空，等待生产者生产\n");
            assert_x(m_lock.wait() == 0);
        }

        int val = m_queue.front();
        m_queue.pop_front();

        LOGI("consumer: 消费 %d\n", val);

        if (m_queue.size() == 9) {
            LOGI("consumer: 通知生产者生产\n");
            assert_x(m_lock.notify_all() == 0);
        }

        assert_x(m_lock.unlock() == 0);
        assert_x(m_lock.unlock() == 0);
    }
}


int main()
{
    LOGI("start\n");

    hash = m_lock.hash_code();

    std::thread *threads[20];
    for (int i = 0; i < sizeof(threads) / sizeof(threads[0]); i ++) {
        threads[i] = (i & 1) ? new std::thread(product) : new std::thread(consumer);
    }
    for (auto &t : threads) {
        t->join();
        delete t;
    }
    return 0;
}