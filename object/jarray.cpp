

#include "jarray.h"
#include "jfield.h"
#include "jobject.h"
#include "jclass.h"

#include "../vm/jvm.h"
#include "../utils/log.h"
#include "../class/jclass_file.h"

#include <shared_mutex>

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

    auto &holder = get_class_holder(vm.class_loader);

    auto *klass = vm.method_area.calloc_type<jclass>();
    klass->name = type;
    klass->access_flag = jclass_file::ACC_PUBLIC | jclass_file::ACC_ABSTRACT | jclass_file::ACC_FINAL;
    klass->object = holder.new_class();

    // 不为基本数据类型添加构造函数和类构造函数
    LOGI("primitive type '%s' created\n", type);
    return klass;
}

jclass *jarray::load_array_class(const char *type) {
    LOGI("loading array class '%s'\n", type);

    std::string type_s(type);
    {
        std::shared_lock rd_lock(m_lock);
        auto it = m_classes.find(type_s);
        if (it != m_classes.end()) {
            LOGI("array class '%s' found in cache, return\n", type);
            return it->second;
        }
    }

    std::unique_lock wr_lock(m_lock);
    {
        auto it = m_classes.find(type_s);
        if (it != m_classes.end()) {
            LOGI("array class '%s' found when double check, return\n", type);
            return it->second;
        }
    }

    LOGI("start loading array type '%s'\n", type);

    // 数组要包裹的类型
    jclass *component_type = nullptr;

#define FIND_OR_CREATE_PRIMITIVE_TYPE(NAME) \
    auto it = m_classes.find(NAME);         \
    if (it == m_classes.end()) {            \
        component_type = create_primitive_type(NAME, m_jvm); \
        m_classes[NAME] = component_type;   \
    }                                       \
    else {                                  \
        component_type = it->second;        \
    }

    // 创建最内层的包裹类
    const int index = (int) type_s.find_last_of('[') + 1; // type_s.find_last_not_of('[');

    switch (type[index]) {
        case 'Z': { FIND_OR_CREATE_PRIMITIVE_TYPE("boolean")    break; }
        case 'B': { FIND_OR_CREATE_PRIMITIVE_TYPE("type")       break; }
        case 'C': { FIND_OR_CREATE_PRIMITIVE_TYPE("char")       break; }
        case 'S': { FIND_OR_CREATE_PRIMITIVE_TYPE("short")      break; }
        case 'I': { FIND_OR_CREATE_PRIMITIVE_TYPE("int")        break; }
        case 'J': { FIND_OR_CREATE_PRIMITIVE_TYPE("long")       break; }
        case 'F': { FIND_OR_CREATE_PRIMITIVE_TYPE("float")      break; }
        case 'D': { FIND_OR_CREATE_PRIMITIVE_TYPE("double")     break; }
        default: {
            component_type = m_jvm.class_loader.load_class(type + index);
            break;
        }
    }
#undef FIND_OR_CREATE_PRIMITIVE_TYPE

    if (component_type == nullptr) {
        LOGE("failed to create component_type '%s' of array '%s'\n", type + index, type);
        exit(1);
    }
    LOGI("the component type of array '%s' is '%s'\n", type, component_type->name);

    auto &holder = get_class_holder(m_jvm.class_loader);

    // 从最内层开始遍历，逐渐生成每个数组类
    for (int i = index - 1; i >= 0; --i) {
        LOGI("[%d/%d]: create class '%s' of array '%s'\n", i + 1, index, type + i, type);
        auto *klass = create_primitive_type(type + i, m_jvm);
        klass->super_class = holder.java_lang_Object;
        klass->component_type = component_type;

        // 数组类型要实现 java.io.Serializable 和 java.lang.Cloneable 接口
        // 但不需要真正创建 jmethod
        klass->interface_num = 2;
        klass->interfaces = m_jvm.method_area.calloc_type<jclass*>(2);
        klass->interfaces[0] = holder.java_io_Serializable;
        klass->interfaces[1] = holder.java_lang_Cloneable;

        // 创建继承树
        klass->parent_tree_size = 3;
        klass->parent_tree = m_jvm.method_area.calloc_type<jclass*>(3);
        klass->parent_tree[0] = holder.java_lang_Object;
        klass->parent_tree[1] = holder.java_io_Serializable;
        klass->parent_tree[2] = holder.java_lang_Cloneable;

        m_classes[type + i] = klass;
        component_type = klass;
    }
    LOGI("load array class '%s' finish, result is '%s'\n", type, component_type->name);
    return component_type;
}

jref jarray::new_type_array(const char *type, int length, int ele_size)
{
    if (length < 0) {
        // todo: 抛出到 java 层
        LOGE("the length of array %s is negative: %d\n", type, length);
        exit(1);
    }

    jclass *klass = load_array_class(type);

    jref ref = m_jvm.heap.malloc_bytes(length * ele_size + 2 * (int) sizeof(jint));
    jobject_ptr obj = m_jvm.heap.lock(ref);

    obj->klass = klass;
    ((jsize *)(obj->values))[0] = length;
    ((jsize *)(obj->values))[1] = ele_size;
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

    std::string s;
    s.append("[").append(klass->name);

    return new_type_array(s.c_str(), length, sizeof(jref));
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
    memcpy(((char*) values) + ele_size * start, buff, ele_size * len);
}