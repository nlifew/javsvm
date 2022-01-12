


#ifndef JAVSVM_CON_POOL_H
#define JAVSVM_CON_POOL_H

#include <atomic>
#include <cstdint>
#include <mutex>
#include <new>

#ifndef NDEBUG
#include "../utils/log.h"
#endif
/**
 * pool.h 提供了两种常用的对象复用池，分别是基于链表实现的 linked_pool
 * 和基于数组实现的 array_pool。两种对象池都需要在构造函数里显式指定容量大小。
 * 如果池中没有可复用的对象，则通过 AllocT 指定的内存分配器创建一个新对象，并返回其引用；
 * 如果池的大小已经到达最大容量，后续放进池的对象将直接被析构。
 *
 * linked_pool 通过原子操作代替锁实现了高性能并发。但作为代价的是，其容量不得超过 255，
 * 且需要额外的字段保存下一个节点。相比于使用 std::mutex 的 array_pool，其性能要高出 30% 左右。
 *
 * array_pool 使用传统的锁保证并发性。如果仅是工作在单线程环境下，可以传进一个空的 LockT 类型
 * 避免性能损失。array_pool 的最大容量为 Int.MAX_VALUE，比 linked_pool 要大得多。
 *
 * 不管是 linked_pool 还是 array_pool，obtain() 返回的对象的一定是最后一个通过
 * recycle() 被回收的对象。
 *
 * 比如下面的代码:
 * linked_pool<int> pool(4);
 * int &int_123 = pool.obtain();    // node: nullptr
 * int &int_234 = pool.obtain();    // node: nullptr
 *
 * int_123 = 123; int_234 = 234;
 *
 * pool.recycle(int_123);           // node: 123 -> nullptr
 * pool.recycle(int_234);           // node: 234 -> 123 -> nullptr
 * assert(pool.obtain() == 234);    // node: 123 -> nullptr
 *
 * 另外需要注意的是，凡是通过 obtain() 获取的对象，*必须* 要调用 recycle() 函数
 * 回收，否则会造成内存泄漏。
 *
 */

namespace javsvm
{

template <typename T>
struct linked_pool_node
{
    T obj;
    linked_pool_node<T> *next;

    template<typename ...Args>
    explicit linked_pool_node(Args&& ...args):
            obj(std::forward<Args>(args)...), next(nullptr)
    {
    }

    ~linked_pool_node() noexcept = default;
};

/**
 * 使用链表实现的复用池
 * 线程同步依赖于 value_t 类型的 m_value，是个 8 字节长度的复合类型
 * 其高 56 位是指向 linked_pool_node<T> 的指针的低 56 位，低 8 位为当前池的对象数
 *
 * @tparam T 容器需要包裹的对象类型
 * @tparam AllocT 内存分配器。默认是 std::allocator
 */
template <typename T, typename AllocT = std::allocator<linked_pool_node<T>>>
class linked_pool
{
private:
    using value_t = int64_t;

    std::atomic<value_t> m_value;
    AllocT m_allocator;
    const int m_capacity;

public:
    explicit linked_pool(int8_t size) noexcept:
            m_value(0), m_capacity(size)
    {
    };

    ~linked_pool() noexcept
    {
        clear();
    }

    linked_pool(const linked_pool &) = delete;
    linked_pool operator=(const linked_pool&) = delete;

    int size() const noexcept
    {
        auto value = m_value.load(std::memory_order_acquire);
        return (int) (value & 0xFF);
    }

    template <typename ...Args>
    T& obtain(Args&&... args) noexcept
    {
        value_t last_value, new_value;
        linked_pool_node<T> *node;

        int loop_count = 0;


        do {

            loop_count ++;

            last_value = m_value.load(std::memory_order_acquire);
            int size = (int) (last_value & 0xFF);

            // 如果对象池为空，直接构造新节点
            if (size <= 0) {
                node = m_allocator.allocate(1);
                ::new((void*) node) linked_pool_node<T>(std::forward<Args>(args)...);
//                m_allocator.construct(node, std::forward<Args>(args)...);

                LOGD("obtain: 循环 %d 次 (缓存池为空，直接创建新对象)\n", loop_count);
                return node->obj;
            }

            // 不为空，寻找下个节点，并更新数据
            node = (linked_pool_node<T> *) (last_value >> 8);
            new_value = ((uint64_t) node->next << 8) | (size - 1);
        } while (! m_value.compare_exchange_strong(last_value, new_value));

        node->next = nullptr;

        LOGD("obtain: 循环 %d 次 (获取到缓存，复用之)\n", loop_count);

        return node->obj;
    }

    bool recycle(const T &t) noexcept
    {
        value_t last_value, new_value;
        auto node = (linked_pool_node<T> *) &t;

        int loop_count = 0;

        do {

            loop_count ++;


            last_value = m_value.load(std::memory_order_acquire);
            int size = (int) (last_value & 0xFF);

            // 如果缓存池已经满了，销毁掉这个对象
            if (size >= m_capacity) {
                node->~linked_pool_node();
//                m_allocator.destroy(node);
                m_allocator.deallocate(node, 1);


                LOGD("recycle: 循环 %d 次 (缓存池已满，直接销毁)\n", loop_count);

                return false;
            }

            node->next = (linked_pool_node<T> *) (last_value >> 8);
            new_value = ((uint64_t) node << 8) | (size + 1);
        } while (! m_value.compare_exchange_strong(last_value, new_value));


        LOGD("recycle: 循环 %d 次 (直接回收)\n", loop_count);

        return true;
    }

    void clear() noexcept
    {
        value_t last_value, new_value;
        linked_pool_node<T> *node;


        int loop_count = 0;

        while (true) {


            loop_count ++;

            last_value = m_value.load(std::memory_order_acquire);
            int size = (int) (last_value & 0xFF);

            // 如果对象池为空，直接返回
            if (size <= 0) {

                LOGD("clear: 销毁 %d 个对象\n", loop_count);

                return;
            }

            // 不为空，寻找下个节点，并更新数据
            node = (linked_pool_node<T> *) (last_value >> 8);
            new_value = ((uint64_t) node->next << 8) | (size - 1);

            bool cas = m_value.compare_exchange_strong(last_value, new_value);
            if (cas) {
//                m_allocator.destroy(node);
                node->~linked_pool_node();
                m_allocator.deallocate(node, 1);
            }
        }
    }
};


/**
 * 基于数组实现的对象池
 * @tparam T 容器需要包裹的对象类型
 * @tparam AllocT 内存分配器。默认是 std::allocator
 * @tparam LockT 锁类型，默认是 std::mutex
 *
 */
template <typename T,
        typename AllocT = std::allocator<T>,
        typename LockT = std::mutex>
class array_pool
{
private:
    LockT m_lock;
    AllocT m_allocator;
    volatile int m_size;
    const int m_capacity;
    T **m_data;
public:
    explicit array_pool(int size) noexcept :
            m_size(0),
            m_capacity(size),
            m_data(new T*[size])
    {
    }

    ~array_pool() noexcept
    {
        clear();
        delete[] m_data;
        m_data = nullptr;
    }

    [[nodiscard]]
    int size() const noexcept { return m_size; }

    void clear() noexcept
    {
        m_lock.lock();
        for (int i = 0, z = m_size; i < z; ++i) {
//            m_allocator.destroy(&m_data[i]);
            m_data[i].~T();
            m_allocator.deallocate(&m_data[i], 1);
        }
        m_lock.unlock();
    }

    template <typename ...Args>
    T &obtain(Args&&... args) noexcept
    {
        T *ptr = nullptr;
        m_lock.lock();
        if (m_size > 0) {
            ptr = m_data[--m_size];
        }
        m_lock.unlock();
        if (ptr == nullptr) {
            ptr = m_allocator.allocate(1);
            ::new((void*) ptr) T(std::forward<Args>(args)...);
//            m_allocator.construct(ptr, std::forward<Args>(args)...);
        }
        return *ptr;
    }

    void recycle(T &t)
    {
        m_lock.lock();
        if (m_size < m_capacity) {
            m_data[m_size ++] = &t;
            m_lock.unlock();
            return;
        }
        m_lock.unlock();
        t.~T();
    }
};

} /* namespace javsvm */
#endif /* JAVSVM_CON_POOL_H */
