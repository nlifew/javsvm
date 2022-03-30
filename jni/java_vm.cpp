//
// Created by edz on 2022/1/10.
//

#include "java_vm.h"
#include "../vm/jvm.h"

#include <cstdlib>

namespace jni
{

jint (JNICALL DestroyJavaVM)(JavaVM *) {
    // 销毁虚拟机会导致进程退出
    // todo: dump 下堆栈
    exit(1);
}

jint (JNICALL AttachCurrentThread)(JavaVM *, void **penv, void *) {
    if (penv) *penv = nullptr;

    auto &jvm = javsvm::jvm::get();

    // 如果已经附加过了，返回 -1
    if (jvm.test() != nullptr) {
        return -1;
    }
    auto &env = jvm.attach();
    if (penv) *penv = &env;
    return 0;
}


jint (JNICALL AttachCurrentThreadAsDaemon)(JavaVM *, void **penv, void *) {
    if (penv) *penv = nullptr;

    auto &jvm = javsvm::jvm::get();

    // 如果已经附加过了，返回 -1
    if (jvm.test() != nullptr) {
        return -1;
    }

    javsvm::jvm::attach_info at = {
            .is_daemon = true,
            .vm = nullptr,
    };

    auto &env = jvm.attach(&at);
    if (penv) *penv = &env;
    return 0;
}

jint (JNICALL DetachCurrentThread)(JavaVM *) {
    auto &jvm = javsvm::jvm::get();

    // 如果没有被附加过，返回 -1
    if (jvm.test() == nullptr) {
        return -1;
    }
    jvm.detach();
    return 0;
}

jint (JNICALL GetEnv)(JavaVM *, void **penv, jint) {
    if (penv) *penv = nullptr;

    auto &jvm = javsvm::jvm::get();
    auto *env = jvm.test();

    if (env == nullptr) {
        return -1;
    }
    if (penv) *penv = env;
    return 0;
}
} // namespace jni

static JNIInvokeInterface_ _jni_invoke_interface = {
        .reserved0 = nullptr,
        .reserved1 = nullptr,
        .reserved2 = nullptr,
        .DestroyJavaVM = jni::DestroyJavaVM,
        .AttachCurrentThread = jni::AttachCurrentThread,
        .DetachCurrentThread = jni::DetachCurrentThread,
        .GetEnv = jni::GetEnv,
        .AttachCurrentThreadAsDaemon = jni::AttachCurrentThreadAsDaemon,
};

static JavaVM s_java_vm = {
        .functions = &_jni_invoke_interface,
};

JavaVM *java_vm = &s_java_vm;

