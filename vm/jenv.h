

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
    explicit jenv(jvm *vm, size_t stack_size) noexcept;

    friend class jvm;

public:
    jvm& jvm;

    const int thread_id;
    jstack stack;

    pthread_t tid;

    /**
     * 绑定的 java.lang.Thread 对象
     */
     jref thread;

    ~jenv() = default;
    jenv(const jenv &) = delete;
    jenv &operator=(const jenv &) = delete;
};

} // namespace javsvm


#endif

