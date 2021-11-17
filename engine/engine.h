

#ifndef JAVSVM_ENGINE_H
#define JAVSVM_ENGINE_H

#include "utils/global.h"

namespace javsvm
{

struct jargs
{
    slot_t *m_args;
    slot_t *m_orig;

    jargs(slot_t *args) : m_args(args), m_orig(args)
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
};




jvalue run_java(jmethod *m, jref obj, jargs &args);
    

jvalue run_jni(jmethod *m, jref obj, jargs &args);


jref check_exception();


void throw_err(const void *msg);


void throw_exp(const char *class_name, const void *msg);



};

#endif 
