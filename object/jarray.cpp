

#include "jarray.h"
#include "jfield.h"
#include "jobject.h"
#include "jclass.h"

#include "../vm/jvm.h"
#include "../utils/log.h"
#include "../class/jclass_file.h"

#include <tuple>
#include <unordered_map>

using namespace javsvm;


struct class_holder
{
    jclass *java_lang_Object = nullptr;
    jclass *java_lang_Class = nullptr;
    jmethod *java_lang_Class_init = nullptr;
    jclass *java_lang_Cloneable = nullptr;
    jclass *java_io_Serializable = nullptr;

#define _CHECK_NULL(x)  \
    if (x == nullptr) { \
        LOGE("field [%s] of class_holder is null\n", #x); \
        exit(1);        \
    }

    explicit class_holder(bootstrap_loader &loader) noexcept
    {
        java_lang_Object = loader.load_class("java/lang/Object");
        java_lang_Class = loader.load_class("java/lang/Class");
        java_lang_Cloneable = loader.load_class("java/lang/Cloneable");
        java_io_Serializable = loader.load_class("java/io/Serializable");

        _CHECK_NULL(java_lang_Object)
        _CHECK_NULL(java_lang_Class)
        _CHECK_NULL(java_lang_Cloneable)
        _CHECK_NULL(java_io_Serializable)

        java_lang_Class_init = java_lang_Class->get_method("<init>", "()V");
        _CHECK_NULL(java_lang_Class_init)
    }
#undef _CHECK_NULL

    [[nodiscard]]
    jref new_class() const noexcept { return java_lang_Class->new_instance();  }
};

static class_holder &get_class_holder(bootstrap_loader &loader) {
    static class_holder instance(loader);
    return instance;
}

jsize jarray::get_array_length(jref array)
{
    auto obj = m_jvm.heap.lock(array);
    if (obj == nullptr) {
        // todo: throw null pointer exception
        LOGE("NullPointerException occurred when access the length of array\n");
        exit(1);
    }
    return ((jsize *)(obj->values))[0];
}

static jclass* create_primitive_type(const char *type, jvm &vm)
{
    LOGI("create primitive type '%s'\n", type);

    auto &holder = get_class_holder(vm.bootstrap_loader);

    auto *klass = vm.method_area.calloc_type<jclass>();
    klass->name = type;
    klass->access_flag = jclass_file::ACC_PUBLIC | jclass_file::ACC_ABSTRACT | jclass_file::ACC_FINAL;
    klass->object = holder.new_class();

    // 不为基本数据类型添加构造函数和类构造函数
    LOGI("primitive type '%s' created\n", type);
    return klass;
}

jref jarray::new_type_array(const char *type, int length, int ele_size)
{
    if (length < 0) {
        // todo: 抛出到 java 层
        LOGE("the length of array %s is negative: %d\n", type, length);
        exit(1);
    }

    // 数组类的类加载器和数组所包裹的类的加载器必须相同。对于基本数据类型，
    // 其类加载器就是系统加载器，因此我们直接使用系统的加载器
    jclass *klass = m_jvm.bootstrap_loader.load_class(type);
    if (klass == nullptr) {
        LOGE("obtain primitive type '%s' failed, abort\n", type);
        exit(1);
    }

    jref ref = m_jvm.heap.malloc_bytes(length * ele_size + 2 * (int) sizeof(jint));
    jobject_ptr obj = m_jvm.heap.lock(ref);

    obj->klass = klass;
    ((jint *)(obj->values))[0] = length;
    ((jint *)(obj->values))[1] = ele_size;
    return ref;
}

/**
 * 非基本类型的数组统一继承自 Object[]
 */
jref jarray::new_object_array(jclass *klass, int length)
{
    if (klass == nullptr) {
        // todo: 抛出到 java 层
        LOGE("null pointer exception\n");
        exit(1);
    }
    if (length < 0) {
        // todo: 抛出到 java 层
        LOGE("the length of array '%s' is negative: %d\n", klass->name, length);
        exit(1);
    }
    std::string name_s(klass->name);

    // 判断传进来的是不是基本类型，从而确定 ele_size
    static std::unordered_map<std::string, std::tuple<int, const char*>> primitive_type {
            { "boolean",{ 1, "[Z" }},
            { "byte",   { 1, "[B" }},
            { "char",   { 2, "[C" }},
            { "short",  { 2, "[S" }},
            { "int",    { 4, "[I" }},
            { "float",  { 4, "[F" }},
            { "long",   { 8, "[J" }},
            { "double", { 8, "[D" }},
    };

    int ele_size = (int) sizeof(jref);
    {
        const auto &it = primitive_type.find(name_s);
        if (it != primitive_type.end()) {
            ele_size = std::get<0>(it->second);
            name_s = std::get<1>(it->second);
        }
        else if (name_s[0] == '[') {
            name_s.insert(0, "[");
        }
        else {
            name_s.insert(0, "[L").append(";");
        }
    }

    jclass *array = nullptr;

    // 对于给定的类型 klass，其数组的类加载器必须和 klass 相同
    if (klass->loader == nullptr) {
        array = m_jvm.bootstrap_loader.load_class(name_s.c_str());
    }
    else {
        // todo: 从自定义类加载器中获取 class_loader 实例
    }
    if (array == nullptr) {
        LOGE("failed to create array type of '%s', abort\n", klass->name);
        exit(1);
    }

    jref ref = m_jvm.heap.malloc_bytes(length * ele_size + 2 * (int) sizeof(jint));
    jobject_ptr obj = m_jvm.heap.lock(ref);

    obj->klass = klass;
    ((jint *)(obj->values))[0] = length;
    ((jint *)(obj->values))[1] = ele_size;
    return ref;
}

void jarray::get_array_region(jref array, jsize start, jint len, void *buff)
{
    auto obj = m_jvm.heap.lock(array);
    if (obj == nullptr) {
        // todo: throw null pointer exception
        LOGE("NullPointerException occurred when get array region\n");
        exit(1);
    }

    int length = ((jsize *)(obj->values))[0];
    int ele_size = ((jsize *)(obj->values))[1];
    void *values = ((jsize *)(obj->values)) + 2;

    if (start < 0 || len < 0 || (start + len) > length) {
        // todo: throw array index out of bounds exception
        LOGE("ArrayIndexOutOfBounds start = [%d], len = [%d], array.length = [%d]\n",
             start, len, length);
        exit(1);
    }
    memcpy(buff, ((char*) values) + ele_size * start, ele_size * len);
}

void jarray::set_array_region(jref array, jsize start, jint len, const void *buff)
{
    auto obj = m_jvm.heap.lock(array);
    if (obj == nullptr) {
        // todo: throw null pointer exception
        LOGE("NullPointerException occurred when get array region\n");
        exit(1);
    }

    int length = ((jsize *)(obj->values))[0];
    int ele_size = ((jsize *)(obj->values))[1];
    void *values = ((jsize *)(obj->values)) + 2;

    if (start < 0 || len < 0 || (start + len) > length) {
        // todo: throw array index out of bounds exception
        LOGE("ArrayIndexOutOfBounds start = [%d], len = [%d], array.length = [%d]\n",
             start, len, length);
        exit(1);
    }
    LOGD("set array region: start [%d], len [%d], buff [%d]\n", start, len, *(int*) buff);
    LOGD("array length [%d], ele_size [%d]\n", length, ele_size);

    char s_buff[16];
    std::string s;
    s.append("[");
    for (int i = 0; i < length; i ++) {
        sprintf(s_buff, "%d, ", ((int*) values)[i]);
        s.append(s_buff);
    }
    s.append("]\n");
    LOGI("%s", s.c_str());

    memcpy(((char*) values) + ele_size * start, buff, ele_size * len);

    s.clear();
    s.append("[");
    for (int i = 0; i < length; i ++) {
        sprintf(s_buff, "%d, ", ((int*) values)[i]);
        s.append(s_buff);
    }
    s.append("]\n");
    LOGI("%s", s.c_str());

    LOGI("set array region end\n");
}