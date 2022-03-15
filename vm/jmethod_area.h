


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
    std::atomic<size_t> m_offset;

public:
    static constexpr int DEFAULT_CAPACITY = 1 * 1024 * 1024;

    explicit jmethod_area(int capacity = DEFAULT_CAPACITY);

    ~jmethod_area();

    /**
     * 申请一块长度为 len 的内存
     * @param len 需要的大小，单位是字节。为 0 时返回 nullptr
     * @return 成功时返回指针，失败退出虚拟机。保证返回的指针对齐到 8 字节
     */
    void *malloc_bytes(size_t len);


    template<typename T>
    T* calloc_type()
    {
        T* ptr = (T*) malloc_bytes(sizeof(T));
        return ::new(ptr)T;
    }

    template<typename T>
    T* calloc_type(size_t n)
    {
        if (n == 0) return nullptr;

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
