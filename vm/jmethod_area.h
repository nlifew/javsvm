


#ifndef JAVSVM_JMETHOD_AREA_H
#define JAVSVM_JMETHOD_AREA_H

//#include <new>
#include <atomic>
#include <cstring>

namespace javsvm
{

class jclass;

class jmethod_area
{
private:
    char *m_buff;
    int m_capacity;
    std::atomic<int> m_offset;

public:
    static constexpr int DEFAULT_CAPACITY = 1 * 1024 * 1024;

    explicit jmethod_area(int capacity = DEFAULT_CAPACITY);

    ~jmethod_area();

    void *malloc_bytes(int len);

    template<typename T>
    T* malloc_type(int n = 1)
    {
        return (T*) malloc_bytes(sizeof(T) * n);
    }

    template<typename T>
    T* calloc_type(int n = 1)
    {
        T* ptr = (T*) malloc_bytes(sizeof(T) * n + sizeof(int));
        return ::new(ptr)T[n];
    }
//
//    int save();
//
//    void restore(int id);
};
}


#endif
