

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

void *jvm::jni() const noexcept
{
    return &((jni_reserved *) m_jni_reserved)->vm;
}

jvm &jvm::get() noexcept
{
    static jvm m;
    return m;
}


struct env_wrapper
{
    char buff[sizeof(jenv)]{};
    bool inited = false;
};

static inline env_wrapper &get_env(const jvm *) noexcept
{
    thread_local env_wrapper w;
    return w;
}

jenv *jvm::env(int) const noexcept
{
    auto &e = get_env(this);
    return e.inited ? (jenv*) e.buff : nullptr;
}


jenv& jvm::env() const noexcept
{
    auto &e = get_env(this);
    if (! e.inited) {
        LOGE("no valid jenv instance found, call jvm::attach() on this thread before\n");
        exit(1);
    }
    return *(jenv*) e.buff;
}


jenv& jvm::attach() noexcept
{
    auto &e = get_env(this);
    if (e.inited) {
        LOGE("you can call jvm::attach() only once on one thread\n");
        exit(1);
    }
    new (e.buff) jenv(*this);
    e.inited = true;
    return *(jenv*) e.buff;
}

void jvm::detach() const noexcept
{
    auto &e = get_env(this);
    if (! e.inited) {
        LOGE("this thread has not attached to a jenv instance, call jvm::attach() before\n");
        exit(1);
    }
    ((jenv *) e.buff)->~jenv();
    e.inited = false;
}

