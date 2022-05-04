

#ifndef JAVSVM_JSTACK_H
#define JAVSVM_JSTACK_H

#include "../utils/global.h"

#include <vector>

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
     * 栈底，用来回复栈指针
     */
    int backup = 0;

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

    void lock_if(jref lck) noexcept;

    void unlock() noexcept;
};


struct java_stack_frame: public jstack_frame
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


    template<typename T>
    inline T top_param() noexcept
    {
        return *(T *)(operand_stack - slotof(T));
    }

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
        *operand_stack = 0;
        *(T *) (operand_stack) = t;
        operand_stack += slotof(T);
        *operand_ref_stack ++ = std::is_same<T, jref>::value ? 1 : 0;
        if (slotof(T) == 2) {
            *operand_ref_stack ++ = 0;
        }
    }


    template<typename T>
    inline void load_param(int idx) noexcept
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

class jstack;

struct jni_stack_frame: public jstack_frame
{
    /**
     * 栈
     */
    jstack *stack = nullptr;

    /**
     * 保存局部引用的表
     */
     jref *local_ref_table = nullptr;

     /**
      * 局部引用表的容量
      */
      int local_ref_table_capacity = 16;

      /**
       * 已使用的局部引用的数量
       */
       int local_ref_table_size = 0;

      /**
       * 保证当前局部引用表最少能容纳 capacity 个引用
       * 成功返回 0，失败返回 -1
       */
      int reserve(int capacity) noexcept;

      /**
       * 向局部引用表的末尾添加一个引用
       * 成功返回该引用在引用表中的指针。失败返回 nullptr
       */
      jref* append(jref ref) noexcept
      {
          if (local_ref_table_size >= local_ref_table_capacity
                && reserve(local_ref_table_size * 2) < 0) {
              return nullptr;
          }
          jref &ret = local_ref_table[local_ref_table_size ++] = ref;
          return &ret;
      }

      /**
       * 移除某个引用。成功返回 ref，失败返回 nullptr
       */
      jref remove(jref ref) const noexcept
      {
          // 考虑到手动 remove 的场景发生多在循环里，
          // 而且是对新获取到的引用进行移除，我们就做个优化，倒序遍历
          for (int i = local_ref_table_size; i > -1; --i) {
              if (local_ref_table[i] == ref) { // 可能要替换成 jheap::equals() ?
                  // 我们并不能做任何内存移动工作，这会使得其它 jref 指针失效
                  local_ref_table[i] = nullptr;
                  return ref;
              }
          }
          return nullptr;
      }


      jref remove(int index) const noexcept
      {
          if (index > -1 && index < local_ref_table_size) {
              jref old = local_ref_table[index];
              local_ref_table[index] = nullptr;
              return old;
          }
          return nullptr;
      }

      /**
       * 得到引用在局部引用表中的位置
       * 找到返回相应的 index，失败返回 -1
       */
      int index_of(jref *ptr) const noexcept
      {
          const ssize_t index = ptr - local_ref_table;
          if (index < local_ref_table_size && index > -1) {
              return (int) index;
          }
          return -1;
      }

      /**
       * 强制指定局部变量表为 size 大小。如果 size 比之前的 size 大，
       * 多余的部分用 nullptr 填充；否则删除队列尾的部分。
       * 成功返回旧的 size，失败返回 -1
       */
      int resize(int size) noexcept
      {
          const int old_size = local_ref_table_size;
          if (old_size < size) {
              if (reserve(size) < 0) {
                  return -1;
              }
              memset(local_ref_table + old_size, 0, sizeof(jref) * (size - old_size));
          }
          local_ref_table_size = size;
          return old_size;
      }
};


struct jclass;

struct stack_trace
{
    const char *source = "";
    const char *klass = "";
    const char *method = "";
    int line_number = 0;
    int pc = 0;
};


class jstack
{
private:
    friend class jni_stack_frame;
    static const size_t DEFAULT_STACK_SIZE = 64 * 1024;  /* aka 64k */
    
private:
    jstack_frame *m_top = nullptr;

    size_t m_offset = 0;
    size_t m_capacity = 0;
    char* m_buff = nullptr;

    void *malloc_bytes(int bytes);

//    void recycle_bytes(int bytes);

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
    explicit jstack(size_t capacity) noexcept;

    
    jstack(const jstack &) = delete;
    jstack &operator=(const jstack &) = delete;

    ~jstack() noexcept;

    [[nodiscard]]
    jstack_frame *top() const noexcept { return m_top; }

    /**
     * 弹出顶部的栈帧
     * @return 弹出后新的栈顶
     */
    jstack_frame *pop() noexcept;

    jstack_frame& push(jmethod *m) noexcept;

    [[nodiscard]]
    std::vector<stack_trace> dump() const noexcept;
};
}

#endif

