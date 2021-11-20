

#include <cstdio>
#include <unistd.h>

#include "vm/jvm.h"
#include "object/jobject.h"
#include "object/jlock.h"
#include "object/jclass.h"
#include "object/jfield.h"
#include "object/jmethod.h"
#include "object/jarray.h"


#include <thread>
#include <queue>

using namespace javsvm;

jlock *m_lock = nullptr;
volatile bool m_end = false;

std::queue<int> m_queue;

// the producer
static void func()
{
    jvm::get().attach();
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    for (int i = 0; i < 4; i ++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        LOGI("product %d\n", i);

        m_lock->lock();
        m_queue.push(i);
        if (m_queue.size() == 1) {
            m_lock->notify_all();
        }
        m_end = i == 3;
        m_lock->unlock();
    }

    jvm::get().detach();
}

static void func2()
{
    jvm::get().attach();

    LOGI("线程 q 尝试获取锁\n");
    m_lock->lock();

    LOGI("线程 q 拿到锁了，尝试释放掉\n");

    m_lock->unlock();
    LOGI("线程 q 已释放锁，结束\n");

    jvm::get().detach();
}

int main() {
    chdir("..");
    putenv(strdup("CLASSPATH=test"));

    printf("start\n");

    jvm &vm = jvm::get();
    jenv &env = vm.attach();

    jlock lock;
    m_lock = &lock;

    std::thread t(func);

    printf("the depth of jlock is %d\n", lock.depth());
    lock.lock();
    printf("the depth of jlock is %d\n", lock.depth());
    lock.lock();

    // the consumer
    while (! m_end) {
        while (m_queue.empty()) {
            lock.wait();
        }
        int val = m_queue.front();
        m_queue.pop();
        LOGI("consume %d\n", val);
    }

    printf("the depth of jlock is %d\n", lock.depth());
    lock.unlock();

    // 开另外一个线程，看看能不能获取到锁
    std::thread q(func2);
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));

    printf("the depth of jlock is %d\n", lock.depth());
    lock.unlock();


    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    printf("the depth of jlock is %d\n", lock.depth());

    t.join();
    q.join();
    return 0;
}
