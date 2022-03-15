

#ifndef JAVSVM_JSTACK_H
#define JAVSVM_JSTACK_H

#include "../utils/global.h"

#include <type_traits>

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
     * 局部引用表，记录了局部变量表中的哪个位置存放的是引用
     */
    u1 *variable_ref_table = nullptr;

    /**
     * 引用操作数栈，记录了操作数栈中的哪个位置存放的是引用
     */
    u1 *operand_ref_stack = nullptr;

    /**
     * 引用操作数栈的初始地址，用来恢复引用操作数栈到原始位置
     */
    u1 *operand_ref_stack_orig = nullptr;

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
     * 则会修改 exp_handler_pc，并将此字段置位。此时解释引擎会清空操作数栈，
     * 并重新修正 pc 为 exp_handler_pc
     */
    jref exp = nullptr;

    /**
     * 异常发生时的 pc 指针
     */
    u4 exp_handler_pc = 0;


    /**
     * 当函数被 synchronized 关键字修饰时，会将这个字段置位
     */
    jref lock = nullptr;

    jstack_frame() noexcept = default;
    ~jstack_frame() noexcept = default;
    jstack_frame(const jstack_frame &) = delete;
    jstack_frame &operator=(const jstack_frame&) = delete;


    template <typename T>
    inline T pop_param() noexcept
    {
        operand_stack -= slotof(T);
        T t = *(T *)(operand_stack);
        operand_ref_stack -= slotof(T);
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
        *operand_ref_stack ++ = std::is_same<T, jref>::value ? 1 : 0;
        if (slotof(T) == 2) {
            *operand_ref_stack ++ = 0;
        }
    }


    template<typename T>
    inline T load_param(int idx) noexcept
    {
        push_param(*(T *) (variable_table + idx));
    }

    template<typename T>
    inline void store_param(int idx) noexcept
    {
#ifndef NDEBUG
        variable_table[idx] = 0;
#endif
        variable_ref_table[idx] = std::is_same<T, jref>::value ? 1 : 0;
        *(T *) (variable_table + idx) = pop_param<T>();
    }


    inline void reset_operand_stack() noexcept
    {
        operand_stack = operand_stack_orig;
        operand_ref_stack = operand_ref_stack_orig;
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
    T* alloc(int n)
    {
        auto ptr = (T *)malloc_bytes(n * sizeof(T) + sizeof(int));
#ifdef NDEBUG
        return ::new(ptr) T[n];
#else
        return ::new(ptr) T[n]{};
#endif
    }

    template<typename T>
    T* alloc()
    {
        auto ptr = (T *)malloc_bytes(sizeof(T));
#ifdef NDEBUG
        return ::new(ptr) T;
#else
        return ::new(ptr) T{};
#endif
    }

public:
    explicit jstack(int capacity = DEFAULT_STACK_SIZE) noexcept;

    
    jstack(const jstack &) = delete;
    jstack &operator=(const jstack &) = delete;

    ~jstack() noexcept;

    [[nodiscard]]
    jstack_frame *top() const noexcept { return m_top; }

    jstack_frame *pop() noexcept;

    jstack_frame& push(jmethod *m) noexcept;

};
}

#endif

