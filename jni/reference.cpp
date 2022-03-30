
#include "jni_utils.h"
#include "jni_env.h"
#include <utility>

namespace jni
{

jobject (JNICALL NewGlobalRef)
        (JNIEnv *, jobject obj) {
    safety_area_guard guard;
    javsvm::jref ref = to_object(obj);
    return (jobject) new javsvm::gc_root(ref);
}

void (JNICALL DeleteGlobalRef)
        (JNIEnv *, jobject obj) {
    safety_area_guard guard;
    delete (javsvm::gc_root *) obj;
}

jobject (JNICALL NewLocalRef)
        (JNIEnv *, jobject obj) {
    safety_area_guard guard;
    return to_object(to_object(obj));
}

void (JNICALL DeleteLocalRef)
        (JNIEnv *, jobject obj) {
    safety_area_guard guard;
    *((javsvm::jref *) obj) = nullptr;
}


jint (JNICALL EnsureLocalCapacity)
        (JNIEnv *, jint capacity) {
    auto &stack = javsvm::jvm::get().env().stack;
    auto frame = (javsvm::jni_stack_frame *) stack.top();
    return frame->reserve(capacity);
}

/**
* 我们使用 gc_weak 包装 jref，而不是将 jref 转为弱引用后再用 gc_root 包装
* 主要是避免 jni 层将弱引用引入栈/数组等层，这样所有的 gc_root 都是强引用，
* 弱引用被局限在 java.lang.ref.Reference#value 字段，简化了 gc 处理逻辑，
* 提高了运行效率和开发效率
*/
jweak (JNICALL NewWeakGlobalRef)
        (JNIEnv *, jobject obj) {
    safety_area_guard guard;

    auto ref = to_object(obj);
    return (jobject) new javsvm::gc_weak(ref);
}

void (JNICALL DeleteWeakGlobalRef)
        (JNIEnv *, jweak ref) {
    safety_area_guard guard;
    delete (javsvm::gc_weak *) ref;
}

/* New JNI 1.6 Features */

jobjectRefType (JNICALL GetObjectRefType)
        (JNIEnv *, jobject obj) {
    if (obj == nullptr) {
        return JNIInvalidRefType;
    }
    using namespace javsvm;

    if (gc_root::ref_pool.contains((gc_root *) obj)) {
        return JNIGlobalRefType;
    }
    if (gc_weak::ref_pool.contains((gc_weak *) obj)) {
        return JNIWeakGlobalRefType;
    }
    return JNILocalRefType;
}

jboolean (JNICALL IsSameObject)
        (JNIEnv *, jobject obj1, jobject obj2) {
    safety_area_guard guard;

    auto _1 = to_object(obj1);
    auto _2 = to_object(obj2);

    return javsvm::jheap::equals(_1, _2);
}

}