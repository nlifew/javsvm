//
// Created by edz on 2021/11/18.
//

#include "jenv.h"
#include "../jni/jni.h"
#include "../jni/jni_utils.h"
#include <atomic>

using namespace javsvm;

static std::atomic<int> thread_id_factory(0);


struct jni_reserved
{
    JNINativeInterface_ interface;
    JNIEnv_ env;
};

jenv::jenv(javsvm::jvm &vm) noexcept:
    jvm(vm),
    thread_id(thread_id_factory.fetch_add(1)),
    m_jni_reserved()
{
    static_assert(sizeof(m_jni_reserved) >= sizeof(jni_reserved));

    auto jni = (jni_reserved *) m_jni_reserved;

    init_jni_env(&jni->interface, this);
    jni->env.functions = &jni->interface;
}

void *jenv::jni() const noexcept
{
    return &((jni_reserved *) m_jni_reserved)->env;
}