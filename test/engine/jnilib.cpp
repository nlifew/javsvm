
#include "engine/jnilib.h"
#include "utils/log.h"

struct jclass
{
    const char *name;
};

struct jmethod
{
    const char *name;
    const char *sig;
};

int main()
{
    jclass klass = {
            .name = "jdk/utils/test/Test__",
    };
    jmethod native_func = {
            .name = "native_func",
            .sig = "(Ljava/lang/String;ILjava/lang/Object;[F测试;)V",
    };
   jmethod native_func2 = {
            .name = "native_func",
            .sig = "(Ljava/lang/String;Ljava/lang/Object;[F测试;)V",
    };
    LOGI("%s\n", jni_short_name(klass.name, native_func.name).c_str());
    LOGI("%s\n", jni_short_name(klass.name, native_func2.name).c_str());
    LOGI("%s\n", jni_long_name(klass.name, native_func.name, native_func.sig).c_str());
    LOGI("%s\n", jni_long_name(klass.name, native_func2.name, native_func2.sig).c_str());
}