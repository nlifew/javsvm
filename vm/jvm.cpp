

#include "jvm.h"
#include "../utils/log.h"

//#include <locale.h>

using namespace javsvm;


//jvm::jvm()
//{
//    // setlocale(LC_ALL, "");
//}

jvm &jvm::get()
{
    static jvm vm;
    return vm;
}

static std::unordered_map<const jvm*, jenv*>& thread_local_map()
{
    thread_local std::unordered_map<const jvm*, jenv*> map;
    return map;
}

jenv& jvm::env()
{
    auto &map = thread_local_map();
    const auto &it = map.find(this);
    if (it == map.end()) {
        LOGE("no valid jenv instance found, call jvm::attach() on this thread before\n");
        exit(1);
    }
    return *(it->second);
}


jenv& jvm::attach()
{
    auto &map = thread_local_map();
    auto it = map.find(this);
    if (it != map.end()) {
        LOGE("you can call jvm::attach() only once on one thread\n");
        exit(1);
    }
    auto *env = new jenv(*this);
    map[this] = env;
    return *env;
}

void jvm::detach()
{
    auto &map = thread_local_map();
    auto it = map.find(this);
    if (it == map.end()) {
        LOGE("this thread has not attached to a jenv instance, call jvm::attach() before\n");
        exit(1);
    }
    map.erase(it);
}

