

#ifndef JAVSVM_JENV_H
#define JAVSVM_JENV_H

#include "jstack.h"
#include <cstring>

namespace javsvm
{

class jvm;

class jenv
{
private:
    jstack m_stack;
    jvm& m_jvm;
    int m_thread_id;

    explicit jenv(jvm& vm) noexcept;

    friend class jvm;
public:
    ~jenv() = default;
    jenv(const jenv &) = delete;
    jenv &operator=(const jenv &) = delete;

    jvm& vm() { return m_jvm; }

    int thread_id() const { return m_thread_id; }

    jstack& stack() { return m_stack; }
};

} // namespace javsvm


#endif

