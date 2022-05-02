//
// Created by edz on 2021/11/18.
//

#include "jenv.h"
#include <atomic>
#include <pthread.h>

using namespace javsvm;

static std::atomic<int> thread_id_factory(0);



jenv::jenv(javsvm::jvm *vm, size_t stack_size) noexcept:
    jvm(*vm),
    thread_id(thread_id_factory.fetch_add(1)),
    tid(pthread_self()),
    thread(nullptr),
    stack(stack_size)
{
}