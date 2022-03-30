
#include "jni.h"

namespace jni
{

jint (JNICALL DestroyJavaVM)(JavaVM *vm);

jint (JNICALL AttachCurrentThread)(JavaVM *vm, void **penv, void *args);

jint (JNICALL DetachCurrentThread)(JavaVM *vm);

jint (JNICALL GetEnv)(JavaVM *vm, void **penv, jint version);

jint (JNICALL AttachCurrentThreadAsDaemon)(JavaVM *vm, void **penv, void *args);
}