
//#include "jni_utils.h"
#include "jni_env.h"

namespace jni
{

jobject (JNICALL NewDirectByteBuffer)
        (JNIEnv *env, void *address, jlong capacity) {
    // todo
    return nullptr;
}

void *(JNICALL GetDirectBufferAddress)
        (JNIEnv *env, jobject buf) {
    // todo
    return nullptr;
}

jlong (JNICALL GetDirectBufferCapacity)
        (JNIEnv *env, jobject buf) {
    // todo
    return 0;
}
}