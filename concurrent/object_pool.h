

#ifndef JAVSVM_CON_OBJECT_POOL
#define JAVSVM_CON_OBJECT_POOL

#include <mutex>
#include <queue>
#include <shared_mutex>

namespace javsvm
{

template <
    typename T,
    typename Lock = std::shared_mutex
>
class object_pool
{
public:
    using constructor_t = T* (*)();
    using destructor_t = void (*)(T*);

private:
    Lock m_lock;
    std::queue<T*> m_queue;
    const size_t m_capacity;
    const constructor_t m_constructor;
    const destructor_t m_destructor;

public:

    object_pool(int capacity = 8, 
                    constructor_t constructor = []() -> T* { return new T; }, 
                    destructor_t destructor = [](T *ptr) { delete ptr; }) :
        m_capacity(capacity), 
        m_constructor(constructor), 
        m_destructor(destructor)
    {
    }

    ~object_pool() = default;

    object_pool(const object_pool<T> &) = delete;
    object_pool<T>& operator=(const object_pool<T>&) = delete;

    T* obtain()
    {
        std::lock_guard<Lock> lock(m_lock);


        if (! m_queue.empty()) {
            return m_queue.front();
        }

        lock.unlock();
        return m_constructor();
    }

    void recycle(T *ptr)
    {
        std::lock_guard<Lock> lock(m_lock);

        if (m_queue.size() < m_capacity) {
            m_queue.push(ptr);
            return;
        }

        lock.unlock();
        m_destructor(ptr);
    }
};


};



#endif 