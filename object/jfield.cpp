

#include "jfield.h"
#include "../object/jclass.h"
#include "../engine/engine.h"

using namespace javsvm;


static void calculate_field_size(const char *sig, int *p_mem_size, int *p_slot_num)
{
    int slot_num = 0;
    int mem_size = 0;

    switch (sig[0]) {
        case 'Z':       /* boolean */
        case 'B':       /* byte */
            mem_size = 1;
            slot_num = 1;
            break;
        case 'C':       /* char */
        case 'S':       /* short */
            mem_size = 2;
            slot_num = 1;
            break;
        case 'I':       /* int */
        case 'F':       /* float */
            mem_size = 4;
            slot_num = 1;
            break;
        case 'D':       /* double */
        case 'J':       /* long */
            mem_size = 8;
            slot_num = 2;
            break;
        case '[':       /* array */
        case 'L':       /* object */
            mem_size = sizeof(slot_t);
            slot_num = 1;
            break;
        default:
            LOGE("calculate_field_size: unknown field sig : %s\n", sig);
            break;
    }
    *p_slot_num = slot_num;
    *p_mem_size = mem_size;
}


void jfield::bind(jclass *_clazz, jclass_file *cls, int index)
{
    clazz = _clazz;
    orig = cls->fields + index;
    access_flag = orig->access_flag;
    name = (char *)cls->constant_pool.cast<jclass_const_utf8>(orig->name_index)->bytes;
    sig = (char *)cls->constant_pool.cast<jclass_const_utf8>(orig->descriptor_index)->bytes;

    calculate_field_size(sig, &mem_size, &slot_num);
}




static inline jvalue get0(const void *src, int size) noexcept
{
    jvalue val;
    switch (size) {
        case 1: val.b = *(jbyte *) src; break;
        case 2: val.s = *(jshort *) src ;break;
        case 4: val.i = *(jint *) src; break;
        case 8: val.j = *(jlong *) src; break;
        default: val.j = 0; break;
    }
    return val;
}

jvalue jfield::get(jref obj) const noexcept
{
    if ((access_flag & jclass_field::ACC_STATIC) != 0) {
        // 静态函数，访问类内存
        return get0((char*) clazz->data + mem_offset, mem_size);
    }

    // 成员函数
    auto ptr = jvm::get().heap.lock(obj);
    if (ptr == nullptr) {
        throw_exp("java/lang/NullPointerException", name);
        return { 0 };
    }

    return get0(ptr->values + mem_offset, mem_size);
}

static inline void set0(void *dst, jvalue val, int size) noexcept
{
    switch (size) {
        case 1: *(jbyte *) dst = val.b; break;
        case 2: *(jshort *) dst = val.s; break;
        case 4: *(jint *) dst = val.i; break;
        case 8: *(jlong *) dst = val.j; break;
        default:
            break;
    }
}

void jfield::set(jref obj, jvalue val) const noexcept
{
    if ((access_flag & jclass_field::ACC_STATIC) != 0) {
        // 静态函数，访问类内存
        set0((char*) clazz->data + mem_offset, val, mem_size);
        return;
    }

    // 成员函数
    auto ptr = jvm::get().heap.lock(obj);
    if (ptr == nullptr) {
        throw_exp("java/lang/NullPointerException", name);
        return;
    }

    set0(ptr->values + mem_offset, val, mem_size);
}