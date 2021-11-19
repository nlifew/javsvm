//
// Created by edz on 2021/11/18.
//

#include "jenv.h"
#include <atomic>

using namespace javsvm;

static std::atomic<int> thread_id_factory(0);

jenv::jenv(jvm &vm) noexcept:
    m_jvm(vm),
    m_thread_id(thread_id_factory.fetch_add(1))
{
}