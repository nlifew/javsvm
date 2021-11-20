
#ifndef JAVSVM_CON_MAP_H
#define JAVSVM_CON_MAP_H

#include <unordered_map>
#include <cstdlib>
#include <shared_mutex>


namespace javsvm
{



template <
    typename Key,
    typename Value,
    typename Lock = std::shared_mutex,
    typename Hash = std::hash<Key>,
    typename Comp = std::equal_to<Key>,
    typename Alloc = std::allocator<std::pair<const Key, Value>>
    >
class concurrent_map
{
private:

    class segment 
    {
    private:
        Lock m_lock;
        std::unordered_map<Key, Value, Hash, Comp, Alloc> m_bucket;
    
    public:
        explicit segment() = default;
        ~segment() = default;

/*
        segment(const segment& o)
        {
            operator=(o);
        }

        segment& operator=(const segment& o)
        {
            m_lock.lock();
            o.m_lock.lock();

            m_segment = o.m_segment;

            o.m_lock.unlock();
            m_lock.unlock();

            return *this;
        }
*/
        const Value* get(const Key &key) noexcept
        {
            std::shared_lock lck(m_lock);
            const auto& it = m_bucket.find(key);
            return it == m_bucket.end() ? nullptr : &(it->second);
        }

        bool contains(const Key &key) const noexcept
        {
            std::shared_lock<Lock> lck(m_lock);
            return m_bucket.find(key) == m_bucket.end();
        }
        
        [[nodiscard]]
        bool empty() const noexcept
        {
            std::shared_lock<Lock> lck(m_lock);
            return m_bucket.empty();
        }

        [[nodiscard]]
        int size() const noexcept
        {
            std::shared_lock<Lock> lck(m_lock);
            return m_bucket.size();
        }

        Value& put(const Key &key, const Value &value) noexcept
        {
            std::unique_lock<Lock> lck(m_lock);
            return m_bucket[key] = value;
        }

        void remove(const Key &key) noexcept
        {
            std::unique_lock<Lock> lck(m_lock);
            m_bucket.erase(key);
        }
        
        void clear() noexcept
        {
            std::unique_lock<Lock> lck(m_lock);
            m_bucket.clear();
        }

        template <typename Func>
        Value& put_if_absent(const Key& key, Func func) noexcept
        {
            {
                std::shared_lock<Lock> lck(m_lock);
                const auto& it = m_bucket.find(key);
                if (it != m_bucket.end()) {
                    return it->second;
                }
            }
            
            std::unique_lock<Lock> lck(m_lock);
            const auto& it = m_bucket.find(key);

            if (it != m_bucket.end()) {
                return it->second;
            }

            return m_bucket[key] = func();
        }
    };


    const int m_segment_count;

    segment *m_segment;

    segment& segment_of(const Key &key) const noexcept
    {
        size_t hash = Hash().operator()(key);
        hash ^= (hash >> 32);

        return m_segment[hash & (m_segment_count - 1)];
    }

    static int segment_size_for(int c) noexcept
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

    using concurrent_map_t = concurrent_map<Key, Value, Lock, Hash, Comp, Alloc>;

    explicit concurrent_map(int segment_count = 4) : 
        m_segment_count(segment_size_for(segment_count)),
        m_segment(new segment[m_segment_count])
    {
    }

    concurrent_map(const concurrent_map_t&) = delete;
    concurrent_map_t& operator=(const concurrent_map_t&) = delete;


    ~concurrent_map()
    {
        delete[] m_segment;
    }

    Value& put(const Key& key, const Value& value) noexcept
    {
        auto& segment = segment_of(key);
        return segment.put(key, value);
    }

    const Value* get(const Key& key) const noexcept
    {
        auto& segment = segment_of(key);
        return segment.get(key);
    }

    void remove(const Key& key) noexcept
    {
        auto& segment = segment_of(key);
        segment.remove(key);
    }

    bool contains_key(const Key& key) const noexcept
    {
        auto& segment = segment_of(key);
        return segment.contains_key(key);
    }

    
    template <typename Func>
    Value& put_if_absent(const Key& key, Func func) noexcept
    {
        auto& segment = segment_of(key);
        return segment.put_if_absent(key, func);
    }

    [[nodiscard]]
    bool empty() const noexcept
    {
        for (int i = 0; i < m_segment_count; i ++) {
            if (! m_segment[i].empty()) return false;
        }
        return true;
    }

    [[nodiscard]]
    int size() const noexcept
    {
        int _size = 0;
        for (int i = 0; i < m_segment_count; i ++) {
            _size += m_segment[i].size();
        }
        return _size;
    }

    void clear() noexcept
    {
        for (int i = 0; i < m_segment_count; i ++) {
            m_segment[i].clear();
        }
    }
};
}

#endif 