
static jsize (JNICALL GetArrayLength)
        (JNIEnv *, jarray array)
{
    return javsvm::jvm::get().array.get_array_length(to_object(array));
}


static jobjectArray (JNICALL NewObjectArray)
        (JNIEnv *, jsize len, jclass clazz, jobject init)
{
    auto _clazz = to_class(clazz);

    if (_clazz == nullptr) {
        return nullptr;
    }

    auto &jvm = javsvm::jvm::get();
    auto array = jvm.array.new_object_array(_clazz, len);

    // 有可能创建失败，故判断一下空指针
    if (array != nullptr) {
        // 如果初始化参数不为 nullptr，创建个临时数组
        auto _obj = to_object(init);
        if (_obj != nullptr && jvm.heap.lock(_obj) != nullptr) {
            auto buff = new javsvm::jref [len];
            std::unique_ptr<javsvm::jref, void(*)(const javsvm::jref*)> buff_guard(
                    buff, [](const javsvm::jref *ptr) { delete[] ptr; });

            for (int i = 0; i < len; i ++) {
                buff[i] = _obj;
            }
            jvm.array.set_object_array_region(array, 0, len, buff);
        }
    }
    return (jobjectArray) to_object(array);
}

static jobject (JNICALL GetObjectArrayElement)
        (JNIEnv *, jobjectArray array, jsize index)
{
    javsvm::jref ref;
    javsvm::jvm::get().array.get_array_region(to_object(array), index, 1, &ref);
    return to_object(ref);
}

static void (JNICALL SetObjectArrayElement)
        (JNIEnv *, jobjectArray array, jsize index, jobject val)
{
    auto _obj = to_object(val);
    javsvm::jvm::get().array.set_array_region(to_object(array), index, 1, _obj);
}


std::mutex s_array_lock;
std::set<javsvm::jobject_ptr*> s_hanging_array;


static void * (JNICALL GetPrimitiveArrayCritical)
        (JNIEnv *, jarray array, jboolean *isCopy)
{
    if (isCopy) *isCopy = false;

    // 总是返回原始数据，而不是拷贝
    auto _obj_ptr = javsvm::jvm::get().heap.lock(to_object(array));
    if (_obj_ptr == nullptr) {
        return nullptr;
    }

    // 阻止数组被回收
    auto copy = new javsvm::jobject_ptr(std::move(_obj_ptr));

    std::unique_lock lck(s_array_lock);
    s_hanging_array.insert(copy);

    return javsvm::jarray::storage_of(*copy);
}

static void (JNICALL ReleasePrimitiveArrayCritical)
        (JNIEnv *, jarray array, void *, jint)
{
    auto _obj_ptr = javsvm::jvm::get().heap.lock(to_object(array));
    if (_obj_ptr == nullptr) {
        return;
    }

    std::unique_lock lck(s_array_lock);
    for (const auto &it : s_hanging_array) {
        if (_obj_ptr == *it) {
            s_hanging_array.erase(it);
            delete it;
            break;
        }
    }
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

