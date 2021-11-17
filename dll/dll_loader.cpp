

#include "dll_loader.h"
#include "dll_wrapper.h"
#include "utils/global.h"

using namespace javsvm;



void dll_loader::load_library(const char *name)
{
    std::lock_guard<std::mutex> lock(m_lock);   // 锁就完事了

    std::string name_s(name);
    auto it = m_cache.find(name_s);

    if (it != m_cache.end()) {
        return;
    }

    dll_wrapper *wrapper = new dll_wrapper(name);
    if (! wrapper->ok()) {
        delete wrapper;
        return;
    } 

    m_cache[name_s] = (wrapper);

    // using JNI_OnLoad_t = jint (*)();
    // todo: 回调 JNI_OnLoad(JavaVM *vm, void*)
}
