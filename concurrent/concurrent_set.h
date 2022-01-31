//
// Created by edz on 2022/1/29.
//

#ifndef JAVSVM_CON_SET_H
#define JAVSVM_CON_SET_H

#include <unordered_set>
#include <shared_mutex>

namespace javsvm
{

template <
    typename E,
    typename Lock,
    typename Hash,
    typename Comp,
    typename Alloc
>
struct concurrent_node
{
private:
    using bucket_type = std::unordered_set<E, Hash, Comp, Alloc>;
    using lock_type = Lock;
    mutable lock_type m_lock;
    bucket_type m_bucket;
public:
    concurrent_node() noexcept = default;
    ~concurrent_node() noexcept = default;

    concurrent_node(const concurrent_node &o) noexcept
    {
        std::shared_lock<lock_type> lck(o.m_lock);
        m_bucket = o.m_bucket;
    }

    concurrent_node& operator=(const concurrent_node &o) noexcept
    {
        std::unique_lock<lock_type> wr_lock(m_lock);
        std::shared_lock<lock_type> rd_lock(o.m_lock);
        m_bucket = o.m_bucket;
        return *this;
    }


    [[nodiscard]]
    int size() const noexcept
    {
        std::shared_lock<lock_type> lck(m_lock);
        return m_bucket.size();
    }

    [[nodiscard]]
    bool empty() const noexcept
    {
        std::shared_lock<lock_type> lck(m_lock);
        return m_bucket.empty();
    }

    const E* contains(const E& val) const noexcept
    {
        std::shared_lock<lock_type> lck(m_lock);
        const auto &it = m_bucket.find(val);
        return it == m_bucket.end() ? nullptr : &it.operator*();
    }

    bool add(const E& val) noexcept
    {
        std::unique_lock<lock_type> lck(m_lock);
        return m_bucket.insert(val).second;
    }

    bool remove(const E& val) noexcept
    {
        std::unique_lock<lock_type> lck(m_lock);
        return m_bucket.erase(val) != 0;
    }

    void clear() noexcept
    {
        std::unique_lock<lock_type> lck(m_lock);
        m_bucket.clear();
    }
};


template <
        typename E,
        typename Lock = std::shared_mutex,
        typename Hash = std::hash<E>,
        typename Comp = std::equal_to<E>,
        typename Alloc = std::allocator<E>
>
class concurrent_set
{
private:
    using node_type = concurrent_node<E, Lock, Hash, Comp, Alloc>;

    const int m_node_count;
    node_type *m_node;

    inline node_type& node_of(const E& val) const noexcept
    {
        size_t hash = Hash().operator()(val);
        hash ^= (hash >> 32);
        return m_node[hash & (m_node_count - 1)];
    }

    static inline int size_for(int c) noexcept
    {
        unsigned n = c - 1;
        n |= n >> 1;
        n |= n >> 2;
        n |= n >> 4;
        n |= n >> 8;
        n |= n >> 16;
        n ++;
        return n <= 0 ? 1 : (int) n;
    }

public:
    explicit concurrent_set(int node_count = 4) noexcept:
        m_node_count(node_count),
        m_node(new node_type[size_for(node_count)])
    {
    }

    concurrent_set(const concurrent_set&) = delete;
    concurrent_set& operator=(const concurrent_set&) = delete;

    ~concurrent_set() noexcept
    {
        delete[] m_node;
    }


    const E* contains(const E& val) const noexcept
    {
        auto &node = node_of(val);
        return node.contains(val);
    }

    bool add(const E& val) noexcept
    {
        auto &node = node_of(val);
        return node.add(val);
    }

    bool remove(const E& val) noexcept
    {
        auto &node = node_of(val);
        return node.remove(val);
    }

    void clear() noexcept
    {
        for (int i = 0; i < m_node_count; ++i) {
            m_node[i].clear();
        }
    }

    [[nodiscard]]
    int size() const noexcept
    {
        int _size = 0;
        for (int i = 0; i < m_node_count; ++i) {
            _size += m_node[i].size();
        }
    }

    [[nodiscard]]
    bool empty() const noexcept
    {
        for (int i = 0; i < m_node_count; ++i) {
            if (! m_node[i].empty()) {
                return false;
            }
        }
        return true;
    }
};

}




#endif //JAVSVM_CON_SET_H
