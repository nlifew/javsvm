

#ifndef JAVSVM_CON_OBJECT_POOL
#define JAVSVM_CON_OBJECT_POOL

#include <mutex>
#include <queue>

namespace javsvm
{

template <
    typename T,
    typename Lock = std::mutex
>
class object_pool
{
public:
    using constructor_t = T* (*)();
    using destructor_t = void (*)(T*);

private:
    Lock m_lock;
    T** m_array;
    volatile int m_size = 0;
    const int m_capacity;
    const constructor_t m_constructor;
    const destructor_t m_destructor;
public:

    explicit object_pool(
            int capacity = 8,
            constructor_t constructor = []() -> T* { return new T(); },
            destructor_t destructor = [](T *ptr) { delete ptr; }
            ) :
        m_array(new T*[capacity]),
        m_capacity(capacity), 
        m_constructor(constructor), 
        m_destructor(destructor)
    {
    }

    ~object_pool()
    {
        while (m_size) {
            m_destructor(m_array[--m_size]);
        }
        delete[] m_array;
        m_size = 0;
        m_array = nullptr;
    }

    object_pool(const object_pool<T> &) = delete;
    object_pool<T>& operator=(const object_pool<T>&) = delete;

    T* obtain()
    {
        {
            std::lock_guard<Lock> lock(m_lock);
            if (m_size > 0) {
                return m_array[-- m_size];
            }
        }

        return m_constructor();
    }

    void recycle(T *ptr)
    {
        {
            std::lock_guard<Lock> lock(m_lock);
            if (m_size < m_capacity) {
                m_array[m_size ++] = ptr;
                return;
            }
        }
        m_destructor(ptr);
    }
};


};



#endif 