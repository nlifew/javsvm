

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

    explicit jenv(jvm& vm) noexcept: m_jvm(vm)
    {
    }

    friend class jvm;
public:
    ~jenv() = default;
    jenv(const jenv &) = delete;
    jenv &operator=(const jenv &) = delete;

    jvm& vm() { return m_jvm; }

    jstack& stack() { return m_stack; }
};

} // namespace javsvm


#endif

