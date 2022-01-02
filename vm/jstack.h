

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
     * 原始的操作数栈指针，用来恢复操作数栈到原始位置
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
    u4 pc = 0;

    /**
     * 栈帧使用的字节数
     */ 
    int bytes = 0;

    /**
     * 异常引用
     * 当异常实例被抛出，虚拟机会进行栈回溯，如果该异常能够被正常捕获，
     * 则会修改 exp_catch_pc，并将此字段置位。此时解释引擎会清空操作数栈，
     * 并重新修正 pc
     */
     jref exp = nullptr;

     u4 exp_handler_pc = 0;

    jstack_frame() = default;
    ~jstack_frame() = default;
    jstack_frame(const jstack_frame &) = delete;
    jstack_frame &operator=(const jstack_frame&) = delete;


    template <typename T>
    inline T pop_param() noexcept
    {
        operand_stack -= slotof(T);
        T t = *(T *)(operand_stack);
        return t;
    }

    template <typename T>
    inline void push_param(const T &t) noexcept
    {
#ifndef NDEBUG
        *operand_stack = 0;
#endif
        *(T *) (operand_stack) = t;
        operand_stack += slotof(T);
    }
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
    T* calloc_type(int n)
    {
        auto ptr = (T *)malloc_bytes(n * sizeof(T) + sizeof(int));
        return ::new(ptr) T[n];
    }

    template<typename T>
    T* calloc_type()
    {
        auto ptr = (T *)malloc_bytes(sizeof(T));
        return ::new(ptr) T;
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

