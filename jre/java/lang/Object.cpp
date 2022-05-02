
#include "jni/jni_utils.h"
#include "vm/jvm.h"
#include "engine/engine.h"

#include <string>

extern "C" JNIEXPORT jobject JNICALL
Java_java_lang_Object_clone
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;

    auto java_lang_Cloneable = javsvm::jvm::get().bootstrap_loader
            .load_class("java/lang/Cloneable");

    if (java_lang_Cloneable == nullptr) {
        javsvm::throw_exp("java/lang/RuntimeException",
                          "failed to load Ljava/lang/Cloneable;, abort\n");
        return nullptr;
    }

    auto object = javsvm::jheap::cast(to_object(self));

    if (! java_lang_Cloneable->is_instance(to_object(self))) {
        std::string msg;
        msg += "class '";
        msg += object->klass->name;
        msg += "' is NOT a instance of Cloneable, abort\n";

        javsvm::throw_exp("java/lang/CloneNotSupportedException",
                          msg.c_str());
        return nullptr;
    }

    // jheap::alloc() 函数的 size 参数不包括填充数据和对象头，因此要减去
    size_t size = object->size() - sizeof(javsvm::jobject);

    auto clone = javsvm::jvm::get().heap.alloc(object->klass, size);
    if (clone == nullptr) {
        return nullptr;
    }

    memcpy(javsvm::jheap::cast(clone)->values, object->values, size);
    return to_object(clone);
}


extern "C" JNIEXPORT jclass JNICALL
Java_java_lang_Object_getClass
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;
    auto object = javsvm::jheap::cast(to_object(self));
    return to_object<jclass>(object->klass->object.get());
}


extern "C" JNIEXPORT void JNICALL
Java_java_lang_Object_notify
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;
    auto object = javsvm::jheap::cast(to_object(self));
    auto ok = object->notify_one();
    if (ok != 0) {
        javsvm::throw_exp("java/lang/IllegalMonitorStateException",
                          " Does this thread have the monitor of this object ?");
    }
}



extern "C" JNIEXPORT void JNICALL
Java_java_lang_Object_notifyAll
    (JNIEnv *, jobject self)
{
    safety_area_guard guard;
    auto object = javsvm::jheap::cast(to_object(self));
    auto ok = object->notify_all();
    if (ok != 0) {
        javsvm::throw_exp("java/lang/IllegalMonitorStateException",
                          " Does this thread have the monitor of this object ?");
    }
}


extern "C" JNIEXPORT void JNICALL
Java_java_lang_Object_wait
    (JNIEnv *, jobject self, jlong timeout)
{
    safety_area_guard guard;
    auto object = javsvm::jheap::cast(to_object(self));
    auto ok = object->wait(timeout);
    if (ok != 0) {
        javsvm::throw_exp("java/lang/IllegalMonitorStateException",
                          " Does this thread have the monitor of this object ?");
    }
}


