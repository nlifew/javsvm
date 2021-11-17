//
// Created by edz on 2021/11/14.
//

#include "recursive_lock.h"
#include "../utils/log.h"
#include <cstdlib>

using namespace javsvm;

recursive_lock::recursive_lock() noexcept:
        m_lock(),
        m_cond(),
        m_reader_count(0)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);

    if (pthread_mutex_init(&m_lock, &attr) != 0) {
        PLOGE("failed to init pthread_mutex\n");
        exit(1);
    }
    pthread_mutexattr_destroy(&attr);

    if (pthread_cond_init(&m_cond, nullptr) != 0) {
        PLOGE("failed to init pthread_cond\n");
        pthread_mutex_destroy(&m_lock);
        exit(1);
    }
}

recursive_lock::~recursive_lock() noexcept
{
    if (pthread_cond_destroy(&m_cond) != 0) {
        PLOGE("failed to destroy pthread_cond\n");
        exit(1);
    }
    if (pthread_mutex_destroy(&m_lock) != 0) {
        PLOGE("failed to destroy pthread_mutex\n");
        exit(1);
    }
}


struct pthread_lock_guard
{
    pthread_mutex_t &m_lock;

    explicit pthread_lock_guard(pthread_mutex_t &lock) noexcept :
        m_lock(lock)
    {
        if (pthread_mutex_lock(&m_lock) != 0) {
            PLOGE("failed to call pthread_mutex_lock\n");
            exit(1);
        }
    }

    pthread_lock_guard(const pthread_lock_guard&) = delete;
    pthread_lock_guard& operator=(const pthread_lock_guard&) = delete;

    ~pthread_lock_guard() noexcept
    {
        if (pthread_mutex_unlock(&m_lock) != 0) {
            PLOGE("failed to call pthread_mutex_unlock\n");
            exit(1);
        }
    }
};



void recursive_lock::lock_shared() noexcept
{
    pthread_lock_guard lock(m_lock);
    m_reader_count ++;
}

bool recursive_lock::try_lock_shared() noexcept
{
    if (pthread_mutex_trylock(&m_lock) == 0) {
        m_reader_count ++;
        return true;
    }
    return false;
}


void recursive_lock::unlock_shared() noexcept
{
    pthread_lock_guard lock(m_lock);
    m_reader_count --;
    if (m_reader_count == 0 && pthread_cond_broadcast(&m_cond) != 0) {
        PLOGE("failed to call pthread_cond_wait\n");
        exit(1);
    }
}


void recursive_lock::lock() noexcept
{
    if (pthread_mutex_lock(&m_lock) != 0) {
        PLOGE("failed to call pthread_mutex_lock\n");
        exit(1);
    }

    while (m_reader_count != 0) {
        if (pthread_cond_wait(&m_cond, &m_lock) != 0) {
            PLOGE("failed to call pthread_cond_wait\n");
            exit(1);
        }
    }
}


bool recursive_lock::try_lock() noexcept
{
    if (pthread_mutex_trylock(&m_lock) == 0 && m_reader_count == 0) {
        return true;
    }
    return false;
}

void recursive_lock::unlock() noexcept
{
    if (pthread_mutex_unlock(&m_lock) != 0) {
        PLOGE("failed to call pthread_mutex_unlock\n");
        exit(1);
    }
}