

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
struct jstack_frame
{
    /**
     * 局部变量表
     */ 
    slot_t *variable_table = nullptr;


    /**
     * 操作数栈
     */ 
    slot_t *operand_stack = nullptr;

    /**
     * 原始的操作数栈指针，用来调试的
     */
     slot_t *operand_stack_orig = nullptr;

    /**
     * 正在执行的函数
     */ 
    jmethod *method = nullptr;


    /**
     * 链表结构，指向下一个栈帧
     */ 
    jstack_frame *next = nullptr;


    /**
     * 指针计数器 pointer counter
     */ 
    int pc = 0;


    /**
     * 栈帧使用的字节数
     */ 
    int bytes = 0;

    jstack_frame() = default;
    ~jstack_frame() = default;
    jstack_frame(const jstack_frame &) = delete;
    jstack_frame &operator=(const jstack_frame&) = delete;
};

class jstack
{
private:
    static const int DEFAULT_STACK_SIZE = 64 * 1024;  /* aka 64k */
    
private:
    jstack_frame *m_top = nullptr;

    int m_offset = 0;
    int m_capacity = 0;
    char* m_buff = nullptr;

    void *malloc_bytes(int bytes);

    void recycle_bytes(int bytes);

    template <typename T>
    T* calloc_type(int n = 1)
    {
        auto ptr = (T *)malloc_bytes(n * sizeof(T) + sizeof(int));
        return ::new(ptr) T[n];
    }

public:
    explicit jstack(int capacity = DEFAULT_STACK_SIZE);

    
    jstack(const jstack &) = delete;
    jstack &operator=(const jstack &) = delete;

    ~jstack();

    [[nodiscard]]
    jstack_frame *top() const noexcept { return m_top; }

    void pop();

    jstack_frame& push(jmethod *m);

};
}

#endif

