

#ifndef JAVSVM_JOBJECT_H
#define JAVSVM_JOBJECT_H

#include <atomic>
#include "../utils/global.h"
//#include "jlock.h"

namespace javsvm
{

struct jclass;
struct jlock;

struct jobject
{
private:
    std::atomic<bool> m_barrier;
    volatile u2 m_owner_thread_count = 0;
    volatile u4 m_hash_count = 0;
    volatile jlock *m_lock = nullptr;
public:
    jclass *klass = nullptr;
    char values[0];

    void lock();

    void unlock();

    void wait(long time_millis = 0);

    void notify();

    void notify_all();
};

} // namespace javsvm


#endif
