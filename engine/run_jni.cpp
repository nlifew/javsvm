
/**
 * native 函数执行逻辑
 */

#include "engine.h"
#include "../utils/global.h"

using namespace javsvm;

jvalue javsvm::run_jni(jmethod *m, jref _this, jargs &args)
{
    // todo: 实现 jni 函数调用
    jvalue v = {0};
    return v;
}