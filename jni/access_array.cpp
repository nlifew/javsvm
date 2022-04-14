
#include "jni_utils.h"
#include "jni_env.h"

namespace jni
{

jsize (JNICALL GetArrayLength)
        (JNIEnv *, jarray array) {
    safety_area_guard guard;
    return javsvm::jvm::get().array.get_array_length(to_object(array));
}


jobjectArray (JNICALL NewObjectArray)
        (JNIEnv *, jsize len, jclass clazz, jobject init) {
    safety_area_guard guard;

    auto _clazz = to_class(clazz);
    if (_clazz == nullptr) {
        return nullptr;
    }

    auto &jvm = javsvm::jvm::get();
    auto array = jvm.array.new_object_array(_clazz, len);

    // 有可能创建失败，故判断一下空指针
    if (array != nullptr) {
        auto _obj = to_object(init);

        if (_obj != nullptr) {
            int ele_size = 0;
            auto *raw = (javsvm::jref *) javsvm::jarray::storage_of(
                    javsvm::jheap::cast(array), &len, &ele_size);
            for (int i = 0; i < len; ++i) {
                raw[i] = _obj;
            }
        }
    }
    return to_object<jobjectArray>(array);
}

jobject (JNICALL GetObjectArrayElement)
        (JNIEnv *, jobjectArray array, jsize index) {
    safety_area_guard guard;

    javsvm::jref ref = nullptr;
    javsvm::jvm::get().array.get_array_region(to_object(array), index, 1, &ref);
    return to_object(ref);
}

void (JNICALL SetObjectArrayElement)
        (JNIEnv *, jobjectArray array, jsize index, jobject val) {
    safety_area_guard guard;

    auto _obj = to_object(val);
    javsvm::jvm::get().array.set_array_region(to_object(array), index, 1, _obj);
}

//static std::atomic<int> m_primitive_counter = 0;

void *(JNICALL GetPrimitiveArrayCritical)
        (JNIEnv *, jarray array, jboolean *isCopy) {
    // 总是返回原始数据，而不是拷贝
    if (isCopy) *isCopy = false;

    safety_area_guard guard;

    auto obj = javsvm::jheap::cast(to_object(array));
    if (obj == nullptr) {
        return nullptr;
    }

    // 这里应该有个判断，如果 array 是对象数组，直接返回 nullptr
    // 对象数组的包裹类型是 jref, jobject 是 jref*，二者是绝对不能混合在一起的
    if (obj->klass->name[1] == '[' || obj->klass->name[1] == 'L') {
        return nullptr;
    }

    // 阻止数组被回收
    javsvm::freeze_gc_thread();

    int len, ele_size;
    return javsvm::jarray::storage_of(obj, &len, &ele_size);
}

void (JNICALL ReleasePrimitiveArrayCritical)
        (JNIEnv *, jarray array, void *, jint) {
    safety_area_guard guard;

    auto obj = javsvm::jheap::cast(to_object(array));
    if (obj == nullptr) {
        return;
    }
    if (obj->klass->name[1] == '[' || obj->klass->name[1] == 'L') {
        return;
    }
    javsvm::unfreeze_gc_thread();
}


#define name Boolean
#define type boolean

#include "access_array_gen.h"


#define type byte
#define name Byte
#include "access_array_gen.h"


#define type char
#define name Char
#include "access_array_gen.h"


#define type short
#define name Short
#include "access_array_gen.h"


#define type int
#define name Int
#include "access_array_gen.h"


#define type long
#define name Long
#include "access_array_gen.h"


#define type float
#define name Float
#include "access_array_gen.h"


#define type double
#define name Double
#include "access_array_gen.h"

}