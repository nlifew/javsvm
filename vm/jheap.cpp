//
// Created by edz on 2021/11/14.
//

#include "jheap.h"
#include "../utils/log.h"
#include "../object/jobject.h"
#include "../object/jclass.h"
#include "../engine/engine.h"


using namespace javsvm;

jheap::jheap(int min_cap, int max_cap) noexcept:
        m_floating_ptr(0)
{
    m_cur_cap = min_cap;
    m_max_cap = max_cap;

    // 直接申请一块初始内存
    m_buff = (char *) malloc(min_cap);
    if (m_buff == nullptr) {
        PLOGE("jheap: can't malloc %d size memory\n", min_cap);
        exit(1);
    }
}

jheap::~jheap() noexcept
{
    free(m_buff);
    m_buff = nullptr;
}



int jheap::realloc_size(int size)
{
    int increase = 0;

    // 如果当前堆内存的容量还没有最大值的一半，直接调整到最大值的一半
    if (m_cur_cap < m_max_cap / 2) {
        increase = m_max_cap / 2;
    }
    else {
        // 按照等比方式扩容，步长为最大堆内存的 10%.
        // 如果这个步长不到 10MB，直接使用 10MB 的
        increase = std::min(m_max_cap / 10, 10 * 1024 * 1024);
    }

    // 如果默认的扩容大小不满足所需大小，按需要的来
    if (increase < size) increase = size;

    // 如果堆内存已满，返回 -1
    if (m_cur_cap + increase > m_max_cap) {
        LOGE("realloc_size: cur heap size %d, required %d, max %d\n",
             m_cur_cap, increase, m_max_cap);
        return -1;
    }

    char *buff = (char *) realloc(m_buff, increase + m_cur_cap);
    if (buff == nullptr) {
        PLOGE("realloc_size: glibc returns nullptr\n"
              "cur heap size %d, required %d, max %d\n",
              m_cur_cap, increase, m_max_cap);
        return -1;
    }
    m_buff = buff;
    m_cur_cap += increase;
    return 0;
}

jobject *jheap::fast_alloc(int size) noexcept
{
    const int result = m_floating_ptr.fetch_add(size);

    if (result + size > m_cur_cap) {
        // 失败，重置指针
        m_floating_ptr.fetch_sub(size);
        return nullptr;
    }

    // 分配成功，初始化对象
    memset(m_buff + result, 0, size);
    return new (m_buff + result) jobject;
}

inline jref jheap::bind(jobject *dst, jclass *src)
{
    dst->klass = src;
    if ((src->flag & jclass::FLAG_FINALIZE) != 0) {
        m_finalize_object.push_back(dst);
    }
    return (jref) dst;
}


static inline int align16(int val) noexcept
{
    return ((val - 1) | 15) + 1;
}

jref jheap::alloc(jclass *klass, int size) noexcept
{
    // 优先使用指针碰撞法分配内存
    jobject *fast_ptr;
    const int real_size = align16(size + (int) sizeof(jobject));

    if ((fast_ptr = fast_alloc(real_size)) != nullptr) {
        return bind(fast_ptr, klass);
    }

    // todo: 触发 gc，并等待其完成


    // 重新分配内存，失败则凉凉
    if ((fast_ptr = fast_alloc(real_size)) != nullptr) {
        return bind(fast_ptr, klass);
    }

    // OutOfMemoryException
    // NOTE: 因为此时堆内存已经耗尽了，我们使用保留区域创建异常
    char msg[128];
    snprintf(msg, sizeof(msg), "heap max size: %d, requested %d, aligned %d\n",
             m_max_cap, size, real_size);
    throw_err("java/lang/OutOfMemoryError", msg);
    return nullptr;
}