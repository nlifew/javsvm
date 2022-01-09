
#include "engine/jnilib.h"
#include "utils/log.h"

using namespace javsvm;


int main()
{
    jclass klass = {
            .name = "jdk/utils/test/Test__",
    };
    jmethod native_func = {
            .clazz = &klass,
            .name = "native_func",
            .sig = "(Ljava/lang/String;ILjava/lang/Object;[F测试;)V",
    };
    jmethod native_func2 = {
            .clazz = &klass,
            .name = "native_func",
            .sig = "(Ljava/lang/String;Ljava/lang/Object;[F测试;)V",
    };
    LOGI("%s\n", jni_short_name(&native_func).c_str());
    LOGI("%s\n", jni_short_name(&native_func2).c_str());
    LOGI("%s\n", jni_long_name(&native_func).c_str());
    LOGI("%s\n", jni_long_name(&native_func2).c_str());
}