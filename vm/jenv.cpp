//
// Created by edz on 2021/11/18.
//

#include "jenv.h"
#include "../jni/jni.h"
#include <atomic>

using namespace javsvm;

static std::atomic<int> thread_id_factory(0);


jenv::jenv(javsvm::jvm *vm) noexcept:
    jvm(*vm),
    thread_id(thread_id_factory.fetch_add(1))
{
}