

#include "jvm.h"
#include "../utils/log.h"

using namespace javsvm;


jvm::jvm() noexcept :
        bootstrap_loader(*this),
        array(*this),
        string(*this),
        dll_loader(this)
{
}

//jvm::~jvm() noexcept
//{
    /*
     * 不能依赖在析构函数里执行 wait_for() 实现退出时阻塞
     * 而应该由主线程主动调用 wait_for(). 既然 jvm 实例正在执行
     * 析构函数，说明有的全局对象已经开始析构，甚至析构完成了，可能会有不确定的行为
     */
//    wait_for();
//}



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

    if_unlikely(! local_env.inited) {
        LOGE("no valid jenv instance found, call jvm::attach() on this thread before\n");
        exit(1);
    }
    return *(jenv*) local_env.buff;
}

jvm::attach_info jvm::DEFAULT_ATTACH_INFO = {
        .is_daemon = false,
        .vm = nullptr,
};

template <typename Cmp, typename T>
static size_t fast_erase(std::vector<T> &vct, const T& val) noexcept
{
    for (size_t i = 0, n = vct.size(); i < n; ++ i) {
        if (Cmp()(vct[i], val)) {
            vct[i] = vct[n - 1];
            vct.pop_back();
            return i;
        }
    }
    return -1;
}

jenv& jvm::attach(attach_info *attach_info) noexcept
{
    if (attach_info == nullptr) {
        attach_info = &DEFAULT_ATTACH_INFO;
    }
    if (local_env.inited) {
        LOGE("you can call jvm::attach() only once on one thread\n");
        exit(1);
    }
    auto *env = new (local_env.buff) jenv(this);
    local_env.attach_info = *attach_info;
    local_env.inited = this;

    std::unique_lock lck(m_mutex);
    m_threads.insert(env);

    if (! attach_info->is_daemon) {
        m_active_threads_count ++;
    }
    if (! m_placeholder_threads.empty()) {
        struct Comparator {
            bool operator()(pthread_t p, pthread_t q) const noexcept
            {
                return pthread_equal(p, q);
            }
        };
        fast_erase<Comparator>(m_placeholder_threads, pthread_self());
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

    std::unique_lock lck(m_mutex);
    m_threads.erase((jenv *) local_env.buff);

    if (! local_env.attach_info.is_daemon) {
        m_active_threads_count --;
        if (m_active_threads_count == 0 && m_placeholder_threads.empty()) {
            m_cond.notify_one();
        }
    }
    leave_safety_area();
}

void jvm::placeholder(pthread_t tid) noexcept
{
    std::lock_guard lck(m_mutex);
    m_placeholder_threads.push_back(tid);
}

void jvm::wait_for() noexcept
{
    std::unique_lock lck(m_mutex);
    while (m_active_threads_count != 0 || !m_placeholder_threads.empty()) {
        m_cond.wait(lck);
    }
}


int jvm::all_threads(std::vector<jenv *> *out) noexcept
{
    std::lock_guard lck(m_mutex);
    if (out != nullptr) {
        out->reserve(out->size() + m_threads.size());
        for (const auto &it : m_threads) {
            out->push_back(it);
        }
    }
    return (int) m_threads.size();
}

