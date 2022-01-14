
/**
 * native 函数执行逻辑
 */

#include "engine.h"
#include "jnilib.h"
#include "../utils/global.h"



using namespace javsvm;


static void *find_entrance(jmethod *method)
{
    auto &loader = jvm::get().dll_loader;

    auto short_name = jni_short_name(method);
    LOGI("find_entrance: try to use short name '%s'\n", short_name.c_str());

    auto ptr = loader.find_symbol(short_name.c_str());
    if (ptr != nullptr) {
        return ptr;
    }
    auto long_name = jni_long_name(method);
    LOGI("find_entrance: try to use short name '%s'\n", long_name.c_str());

    return loader.find_symbol(long_name.c_str());
}

// todo: 实现 jni 函数调用
jvalue javsvm::run_jni(jmethod *method, jref obj, jargs &args)
{
    jvalue v = {0};

    if (method->entrance.jni_func == nullptr) {
        // 如果当前函数还没有绑定，尝试去动态库中找
        auto ptr = find_entrance(method);
        if (ptr == nullptr) {
            // todo 抛出一个链接异常
            LOGE("run_jni: can't find entrance of '%s'->%s%s\n", method->clazz->name,
                 method->name, method->sig);
            exit(1);
        }
        method->entrance.jni_func = ptr;
    }


    return v;
}