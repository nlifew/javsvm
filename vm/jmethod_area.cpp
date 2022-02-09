


#include "jmethod_area.h"
#include "../object/jclass.h"
#include "../utils/log.h"

#include <assert.h>

using namespace javsvm;


jmethod_area::jmethod_area(int capacity) : m_offset(0)
{
    m_capacity = capacity;
    m_buff = new char[capacity];
}

jmethod_area::~jmethod_area()
{
    m_capacity = 0;
    delete[] m_buff;
}


/**
 * 使用指针碰撞法快速分配内存 
 */
void* jmethod_area::malloc_bytes(int bytes)
{
    // 需要先把大小按照 8 字节对齐
     bytes = ((bytes - 1) | 7) + 1;

    if (bytes == 0) {
        return nullptr;
    }

    const int end = (m_offset += bytes);
    if (end > m_capacity) {
        PLOGE("fatal: failed to malloc %d bytes from jmethod_area, [%d/%d]\n", bytes, end - bytes, m_capacity);
        exit(1);
    }
    void *result = m_buff + end - bytes;

    // 保证分配出去的指针满足 8 字节对齐
    assert((((uint64_t) result) & 7) == 0);

    return result;
}
//
//int jmethod_area::save()
//{
//    return m_offset;
//}
//
//void jmethod_area::restore(int id)
//{
//    m_offset = id;
//}