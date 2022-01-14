//
// Created by edz on 2022/1/10.
//

#include "jni.h"
#include "../vm/jvm.h"

#include <cstdlib>

static inline javsvm::jvm * to_jvm(JavaVM *vm) noexcept
{
    return (javsvm::jvm *) vm->functions->reserved0;
}


jint (JNICALL DestroyJavaVM)(JavaVM *vm)
{
    // 销毁虚拟机会导致进程退出
    // todo: dump 下堆栈
    exit(1);
}

jint (JNICALL AttachCurrentThread)(JavaVM *vm, void **penv, void *arg)
{
    if (penv) *penv = nullptr;

    auto *jvm = to_jvm(vm);

    // 如果已经附加过了，返回 -1
    if (jvm->env(0) != nullptr) {
        return -1;
    }
    auto &env = jvm->attach();
    if (penv) *penv = &env;
    return 0;
}

jint (JNICALL DetachCurrentThread)(JavaVM *vm)
{
    auto *jvm = to_jvm(vm);

    // 如果没有被附加过，返回 -1
    if (jvm->env(0) == nullptr) {
        return -1;
    }
    jvm->detach();
    return 0;
}

jint (JNICALL GetEnv)(JavaVM *vm, void **penv, jint version)
{
    if (penv) *penv = nullptr;

    auto *jvm = to_jvm(vm);
    auto *env = jvm->env(0);

    if (env == nullptr) {
        return -1;
    }
    if (penv) *penv = env;
    return 0;
}

jint (JNICALL AttachCurrentThreadAsDaemon)(JavaVM *vm, void **penv, void *args)
{
    // todo
    return -1;
}

static JNIInvokeInterface_ g_invoke_interface = {
        .reserved0 = nullptr,
        .reserved1 = nullptr,
        .reserved2 = nullptr,
        .DestroyJavaVM = ::DestroyJavaVM,
        .AttachCurrentThread = ::AttachCurrentThread,
        .DetachCurrentThread = ::DetachCurrentThread,
        .GetEnv = ::GetEnv,
        .AttachCurrentThreadAsDaemon = ::AttachCurrentThreadAsDaemon,
};

int init_jni_vm(JNIInvokeInterface_ *dst, javsvm::jvm *jvm) noexcept
{
    memcpy(dst, &g_invoke_interface, sizeof(JNIInvokeInterface_));
    dst->reserved0 = jvm;
    return 0;
}
