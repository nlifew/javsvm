

#include "jvm.h"
#include "../utils/log.h"
#include "../jni/jni.h"
#include "../jni/jni_utils.h"

//#include <locale.h>

using namespace javsvm;


//jvm::jvm()
//{
//    // setlocale(LC_ALL, "");
//}

struct jni_reserved
{
    JNIInvokeInterface_ interface;
    JavaVM_ vm;
};

jvm::jvm() noexcept :
        bootstrap_loader(*this),
        array(*this),
        string(*this),
        dll_loader(this),
        m_jni_reserved()
{
    static_assert(sizeof(m_jni_reserved) >= sizeof(jni_reserved));

    auto jni = (jni_reserved *) m_jni_reserved;

    init_jni_vm(&jni->interface, this);
    jni->vm.functions = &jni->interface;
}

jvm::~jvm() noexcept
{
    wait_for();
}


void *jvm::jni() const noexcept
{
    return &((jni_reserved *) m_jni_reserved)->vm;
}



struct env_wrapper
{
    jvm *inited = nullptr;
    jvm::attach_info attach_info{};
    char buff[sizeof(jenv)]{};
};


thread_local env_wrapper local_env;


jenv *jvm::test() const noexcept
{
    (void) this; // suppress static warning
    return local_env.inited ? (jenv*) local_env.buff : nullptr;
}


jenv& jvm::env() const noexcept
{
    (void) this; // suppress static warning

    if (! local_env.inited) {
        LOGE("no valid jenv instance found, call jvm::attach() on this thread before\n");
        exit(1);
    }
    return *(jenv*) local_env.buff;
}

jvm::attach_info jvm::DEFAULT_ATTACH_INFO = {
        .is_daemon = false,
        .vm = nullptr,
};

jenv& jvm::attach(jvm::attach_info &attach_info) noexcept
{
    if (local_env.inited) {
        LOGE("you can call jvm::attach() only once on one thread\n");
        exit(1);
    }
    auto *env = new (local_env.buff) jenv(this);
    local_env.attach_info = attach_info;
    local_env.inited = this;

    const pthread_t tid = pthread_self();

    std::lock_guard lck(m_mutex);
    if (attach_info.is_daemon) {
        // 移除占位 pthread_t
        m_threads.erase(tid);
        if (m_threads.empty()) {
            m_cond.notify_one();
        }
    }
    else {
        m_threads.insert(tid);
    }
    return *env;
}


void jvm::detach() noexcept
{
    if (! local_env.inited) {
        LOGE("this thread has not attached to a jenv instance, call jvm::attach() before\n");
        exit(1);
    }

    ((jenv *) local_env.buff)->~jenv();
    local_env.inited = nullptr;

    if (! local_env.attach_info.is_daemon) {
        std::lock_guard lck(m_mutex);
        m_threads.erase(pthread_self());
        if (m_threads.empty()) {
            m_cond.notify_one();
        }
    }
}

void jvm::placeholder(pthread_t tid) noexcept
{
    std::lock_guard lck(m_mutex);
    m_threads.insert(tid);
}

void jvm::wait_for() noexcept
{
    if (m_threads.empty()) { // 是线程安全的
        return;
    }

    std::unique_lock lck(m_mutex);
    while (! m_threads.empty()) {
        m_cond.wait(lck);
    }
}
