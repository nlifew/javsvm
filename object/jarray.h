

#ifndef JAVSVM_JARRAY_H
#define JAVSVM_JARRAY_H

#include "../utils/global.h"
#include "jobject.h"
#include "../concurrent/recursive_lock.h"

#include <unordered_map>
#include <string>

namespace javsvm
{

class jobject;
struct jclass;
class jvm;

class jarray
{
private:
    jvm &m_jvm;

    jref new_type_array(const char *type, int length, int ele_size) noexcept;
public:
    void get_array_region(jref array, jsize start, jint len, void *buff) const noexcept;

    void set_array_region(jref array, jsize start, jint len, const void *buff) const noexcept;

//    /**
//     * 获取数组存储元素的起始地址
//     */
//    static void *storage_of(jobject_ptr &ptr) noexcept;

    static inline void *storage_of(jobject *array, int *len, int *ele_size) noexcept
    {
        auto *data = (jsize *)(array->values);
        *len = data[0];
        *ele_size = data[1];
        return &data[2];
    }

public:
    explicit jarray(jvm &vm) noexcept :
        m_jvm(vm)
    {
    }
    jarray(const jarray&) = delete;
    jarray& operator=(const jarray&) = delete;

    jsize get_array_length(jref array) const noexcept;

    jref new_boolean_array(int length) { return new_type_array("[Z", length, sizeof(jboolean)); }

    jref new_byte_array(int length) { return new_type_array("[B", length, sizeof(jbyte)); }

    jref new_char_array(int length) { return new_type_array("[C", length, sizeof(jchar)); }

    jref new_short_array(int length) { return new_type_array("[S", length, sizeof(jshort)); }

    jref new_int_array(int length) { return new_type_array("[I", length, sizeof(jint)); }

    jref new_float_array(int length) { return new_type_array("[F", length, sizeof(jfloat)); }

    jref new_long_array(int length) { return new_type_array("[J", length, sizeof(jlong)); }

    jref new_double_array(int length) { return new_type_array("[D", length, sizeof(jdouble)); }

    jref new_object_array(jclass *klass, int length);

    void get_bool_array_region(jref array, jsize start, jsize l, jboolean *buf) { get_array_region(array, start, l, buf); }

    void get_byte_array_region(jref array, jsize start, jsize l, jbyte *buf) { get_array_region(array, start, l, buf); }

    void get_char_array_region(jref array, jsize start, jsize l, jchar *buf) { get_array_region(array, start, l, buf); }

    void get_short_array_region(jref array, jsize start, jsize l, jshort *buf) { get_array_region(array, start, l, buf); }

    void get_int_array_region(jref array, jsize start, jsize l, jint *buf) { get_array_region(array, start, l, buf); }

    void get_float_array_region(jref array, jsize start, jsize l, jfloat *buf) { get_array_region(array, start, l, buf); }

    void get_long_array_region(jref array, jsize start, jsize l, jlong *buf) { get_array_region(array, start, l, buf); }

    void get_double_array_region(jref array, jsize start, jsize l, jdouble *buf) { get_array_region(array, start, l, buf); }

    void get_object_array_region(jref array, jsize start, jsize l, jref *buf) { get_array_region(array, start, l, buf); }


    void set_bool_array_region(jref array, jsize start, jsize len, const jboolean *buf) { set_array_region(array, start, len, buf); }

    void set_byte_array_region(jref array, jsize start, jsize len, const jbyte *buf) { set_array_region(array, start, len, buf); }

    void set_char_array_region(jref array, jsize start, jsize len, const jchar *buf) { set_array_region(array, start, len, buf); }

    void set_short_array_region(jref array, jsize start, jsize len, const jshort *buf) { set_array_region(array, start, len, buf); }

    void set_int_array_region(jref array, jsize start, jsize len, const jint *buf) { set_array_region(array, start, len, buf); }

    void set_float_array_region(jref array, jsize start, jsize len, const jfloat *buf) { set_array_region(array, start, len, buf); }

    void set_long_array_region(jref array, jsize start, jsize len, const jlong *buf) { set_array_region(array, start, len, buf); }

    void set_double_array_region(jref array, jsize start, jsize len, const jdouble *buf) { set_array_region(array, start, len, buf); }

    void set_object_array_region(jref array, jsize start, jsize len, jref *buf) { set_array_region(array, start, len, buf); }
};

} // namespace javsvm


#endif 
