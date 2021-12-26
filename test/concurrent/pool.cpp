//
// Created by edz on 2021/12/26.
//

//#define LOG_LEVEL LOG_LEVEL_DEBUG

#include "concurrent/pool.h"
#include "utils/log.h"

#include <thread>

using javsvm::linked_pool;

static linked_pool<int> m_pool(4);
static std::atomic<int> m_thread_id(0);

static void run()
{
    const int id = m_thread_id ++;
    const int count = (rand() & 31) + 1;

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//    LOGI("线程 %d 开始，循环 %d 次\n", id, count);

    for (int i = 0; i < count; ++i) {
        LOGI("线程 %d 尝试获取对象\n", id);
        auto &it = m_pool.obtain(0);
        LOGI("线程 %d 获取对象成功，\n", id);

        int val = rand() & 65535;
        LOGI("线程 %d 读取到的值为 %d，修改为 %d\n", id, it, val);
        it = val;
        LOGI("线程 %d 尝试释放对象\n", id);
        bool recycled = m_pool.recycle(it);
        LOGI("线程 %d 释放对象结束 %d\n", id, recycled);
    }
}

#define N 20

int main()
{
    srand(0);
    std::thread *threads[N];
    for (auto &it : threads) {
        it = new std::thread(run);
    }
    for (auto it : threads) {
        it->join();
        delete it;
    }
    LOGI("size = %d\n", m_pool.size());
}