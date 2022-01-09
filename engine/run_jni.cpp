
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

    //
}

// todo: 实现 jni 函数调用
jvalue javsvm::run_jni(jmethod *method, jref obj, jargs &args)
{
    jvalue v = {0};

    return v;
}