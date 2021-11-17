

#ifndef JAVSVM_JHEAP_H
#define JAVSVM_JHEAP_H

#include <atomic>
#include "../utils/global.h"

namespace javsvm
{

struct jobject;
class jheap;

class jobject_ptr;


class jheap
{
private:

public:
    jref malloc_bytes(int bytes);

    jobject_ptr lock(jref ref);

    void unlock(jobject *ptr);

    bool is_nullptr(jref ref);
};


class jobject_ptr
{
private:
    jobject *m_ptr;
    jheap &m_heap;
public:
    explicit jobject_ptr(jheap &heap, jobject *ptr = nullptr) noexcept :
            m_heap(heap),
            m_ptr(ptr)
    {
    }


    jobject_ptr(const jobject_ptr&) = delete;

    jobject_ptr& operator=(const jobject_ptr& o) = delete;

    jobject_ptr(jobject_ptr&& o) noexcept:
            m_heap(o.m_heap),
            m_ptr(o.m_ptr)
    {
        o.m_ptr = nullptr;
    }

    jobject_ptr& operator=(jobject_ptr&& o) noexcept
    {
        reset(o.m_ptr);
        o.m_ptr = nullptr;
        return *this;
    }


    ~jobject_ptr() { reset(); }

    bool operator==(const jobject *p) const noexcept { return m_ptr == p; }


    [[nodiscard]]
    jobject* get() const noexcept { return m_ptr; }

    explicit operator bool() const noexcept { return m_ptr != nullptr; }

    jobject* operator*() const noexcept { return m_ptr; }

    jobject* operator->() const noexcept { return m_ptr; }

    void reset(jobject *ptr = nullptr)
    {
        if (m_ptr) {
            m_heap.unlock(m_ptr);
        }
        m_ptr = ptr;
    }
};

}



#endif