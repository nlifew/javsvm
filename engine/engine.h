

#ifndef JAVSVM_ENGINE_H
#define JAVSVM_ENGINE_H

#include "../vm/jstack.h"

namespace javsvm
{

class jargs
{
    const slot_t *m_args;
    const slot_t *m_orig;

public:
    explicit jargs(const slot_t *args) noexcept :
        m_args(args), m_orig(args)
    {
    }

    template <typename T>
    T& next() noexcept 
    {
        T *p = (T *) m_args;
        m_args += slotof(T);
        return *p;
    }

    template <typename T>
    T& prev() noexcept
    {
        m_args -= slotof(T);
        return *(T *)m_args;
    }

    void reset() noexcept { m_args = m_orig; }

    [[nodiscard]]
    const slot_t *begin() const noexcept { return m_orig; }

    [[nodiscard]]
    const slot_t *where() const noexcept { return m_args; }
};



/**
 * 调用某个 java 函数
 * NOTE: run_java 只会执行函数，不会做任何多余的工作。包括但不限于
 * 抽象函数检查，obj 是否为 null，参数列表是否匹配，上锁等。
 */
jvalue run_java(jmethod *m, jref obj, jargs &args);

/**
 * 运行某个 jni 函数
 */
jvalue run_jni(jmethod *m, jref obj, jargs &args);


jref check_exception() noexcept;

/**
 * 清除已经抛出的异常
 * NOTE: 此函数只支持 jni 中调用。在 java 中被调整过的栈是无法被还原的
 * (todo: 当然也是有办法的，待验证，因此先不支持)
 */
void clear_exception() noexcept;

/**
 * 抛出指定的对象
 */
void throw_throwable(jref ref) noexcept;

/**
 * 创建并抛出一个异常. java 中的 Error 类也可以通过此方式被抛出
 * @param class_name 类名，必须以 'L' 开头，以 '/' 作为分隔符
 * @param msg 消息，允许为 nullptr
 */
void throw_exp(const char *class_name, const char *msg);

/**
 * 创建并抛出一个不会被捕获的错误(Error)，用于展示堆栈并退出虚拟机
 * 和 throw_exp 不同，当 java 堆已经耗尽时，会尝试从保留的安全区中创建对象。
 */
void throw_err(const char *class_name, const void *msg);

} // namespace javsvm

#endif 
