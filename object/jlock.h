

#ifndef JAVSVM_JLOCK_H
#define JAVSVM_JLOCK_H

#include <atomic>
#include <concurrent/object_pool.h>

namespace javsvm
{


class jlock
{
private:
    struct lock_struct;

    std::atomic<bool> m_lock;
    volatile int m_waiting_num;
    volatile lock_struct *m_lock_st;

public:
    void lock();

    void unlock();

    void wait(long time);

    void notify();

    void notify_all();
 };
  
} // namespace javsvm


#endif 