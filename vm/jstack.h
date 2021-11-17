

#ifndef JAVSVM_JSTACK_H
#define JAVSVM_JSTACK_H

#include "../utils/global.h"



namespace javsvm
{

struct jmethod;

using slot_t = std::uint64_t;

/**
 * 栈帧
 */ 
class jframe
{
    friend class jstack;
    /**
     * 局部变量表
     */ 
    slot_t *variable_table = nullptr;


    /**
     * 操作数栈
     */ 
    slot_t *operand_stack = nullptr;


    /**
     * 正在执行的函数
     */ 
    jmethod *method = nullptr;


    /**
     * 链表结构，指向下一个栈帧
     */ 
    jframe *next = nullptr;


    /**
     * 指针计数器 pointer counter
     */ 
    u4 pc = 0;


    /**
     * 栈帧使用的字节数
     */ 
    int bytes = 0;

    jframe() = delete;
    ~jframe() = delete;
    jframe(const jframe &) = delete;
    jframe &operator=(const jframe&) = delete;
};

class jstack
{
private:
    static const int DEFAULT_STACK_SIZE = 32 * 1024;  /* aka 32k */
    
private:
    jframe *m_top = nullptr;

    int m_offset = 0;
    int m_capacity = 0;
    char* m_buff = nullptr;

    void *malloc_bytes(int bytes);

    void recycle_bytes(int bytes);

    template <typename T>
    T* calloc_type(int n = 1)
    {
        auto ptr = (T *)malloc_bytes(n * sizeof(T));
        memset(ptr, 0, sizeof(T) * n);
        return ptr;
    }

public:
    explicit jstack(int capacity = DEFAULT_STACK_SIZE);

    
    jstack(const jstack &) = delete;
    jstack &operator=(const jstack &) = delete;

    ~jstack();

    jframe *top() { return m_top; }

    void pop();

    jframe& push(jmethod *m);

};
}

#endif

