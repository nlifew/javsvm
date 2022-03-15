

#ifndef JAVSVM_JENV_H
#define JAVSVM_JENV_H

#include "jstack.h"
#include "../gc/gc_root.h"

namespace javsvm
{
class jvm;

class jenv
{
private:
    explicit jenv(jvm *vm) noexcept;

    friend class jvm;

    /**
     * jni 保留的
     */
    char m_jni_reserved[235 * sizeof(void*)];

    /**
     * 绑定的 java 层 Thread 对象
     */
    gc_root thread = nullptr;

public:
    jvm& jvm;

    const int thread_id;
    jstack stack;

    ~jenv() = default;
    jenv(const jenv &) = delete;
    jenv &operator=(const jenv &) = delete;

    /**
     * 返回 jni 使用的 JNIEnv*
     */
     [[nodiscard]]
     void *jni() const noexcept;
};

} // namespace javsvm


#endif

